// Отмена вычислений: срабатывает, когда просят, и не мешает, когда нет.

#include "geo/boolean.h"
#include "geo/cancel.h"

#include <QTest>

#include <chrono>
#include <stop_token>
#include <thread>

using namespace Geo;
using Clock = std::chrono::steady_clock;

namespace {

// Нагрузка: сетка окружностей, каждая -- отдельный точный свип, а потом
// общее объединение. Размер подобран так, чтобы полный счёт занимал около
// секунды: меньше -- и отмену некуда было бы вклинить, больше -- тест стал
// бы неприлично долгим.
constexpr int gridSide = 56;

Polylines workload() {
    Polylines out;
    for(int i = 0; i < gridSide; ++i)
        for(int j = 0; j < gridSide; ++j) {
            Polyline circle{Vertex(i * 1.5 - 1.0, j * 1.5, 1.0), Vertex(i * 1.5 + 1.0, j * 1.5, 1.0)};
            circle.closed = true;
            circle.width = 0.6;
            out.push_back(std::move(circle));
        }
    return out;
}

double elapsedMs(Clock::time_point since) {
    return std::chrono::duration<double, std::milli>(Clock::now() - since).count();
}

} // namespace

class CancelTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void withoutScopeNothingIsCancelled();
    void scopeWithoutRequestComputesFully();
    void requestBeforeStartThrowsAtOnce();
    void requestMidFlightStopsEarly();
    void scopeExitRestoresPreviousState();
    void nestedScopesRestoreOuterToken();

private:
    Polylines m_src;
    double m_fullRunMs = 0.0; // время неотменённого счёта, база для сравнения
};

void CancelTest::initTestCase() {
    m_src = workload();
    const auto started = Clock::now();
    const Polygons full = Inflate(m_src);
    m_fullRunMs = elapsedMs(started);
    QVERIFY(full.all().size() > 0);
    qInfo("контуров %zu, полный счёт %.0f мс", m_src.size(), m_fullRunMs);
}

void CancelTest::withoutScopeNothingIsCancelled() {
    // Вне всякой области отмена не существует -- вызывающий, которому она
    // не нужна, ничего о ней и не узнаёт.
    QVERIFY(!cancelRequested());
    QVERIFY(!cancelToken().stop_possible());
    checkCancelled(); // не бросает
}

void CancelTest::scopeWithoutRequestComputesFully() {
    std::stop_source stop;
    const CancelScope scope{stop.get_token()};

    QVERIFY(cancelToken().stop_possible());
    QVERIFY(!cancelRequested());
    QVERIFY(Inflate(m_src).all().size() > 0);
}

void CancelTest::requestBeforeStartThrowsAtOnce() {
    // Самая жёсткая проверка -- без всякой зависимости от времени: отмена
    // запрошена ещё до вызова, значит первая же проверка обязана сработать.
    std::stop_source stop;
    const CancelScope scope{stop.get_token()};
    stop.request_stop();

    QVERIFY(cancelRequested());
    QVERIFY_THROWS_EXCEPTION(Cancelled, checkCancelled());

    const auto started = Clock::now();
    QVERIFY_THROWS_EXCEPTION(Cancelled, Inflate(m_src));
    QVERIFY2(elapsedMs(started) < m_fullRunMs / 4,
        "отмена, запрошенная до старта, должна прервать счёт сразу");
}

void CancelTest::requestMidFlightStopsEarly() {
    constexpr int requestAfterMs = 50;

    std::stop_source stop;
    const CancelScope scope{stop.get_token()};
    std::jthread canceller([&stop, requestAfterMs] {
        std::this_thread::sleep_for(std::chrono::milliseconds(requestAfterMs));
        stop.request_stop();
    });

    const auto started = Clock::now();
    QVERIFY_THROWS_EXCEPTION(Cancelled, Inflate(m_src));
    const double took = elapsedMs(started);
    qInfo("отмена через %d мс -> прервано за %.0f мс (полный счёт %.0f мс)",
        requestAfterMs, took, m_fullRunMs);

    // Отмена кооперативна, так что мгновенной она не бывает: сравнение --
    // с полным счётом, а не с абсолютным порогом, иначе тест зависел бы от
    // того, насколько быстра машина.
    QVERIFY2(took < m_fullRunMs / 2, "счёт после отмены шёл слишком долго");
}

void CancelTest::scopeExitRestoresPreviousState() {
    {
        std::stop_source stop;
        const CancelScope scope{stop.get_token()};
        stop.request_stop();
        QVERIFY(cancelRequested());
    }
    // Отменённая область закрылась -- следующий счёт снова обычный.
    QVERIFY(!cancelRequested());
    QVERIFY(Inflate(m_src).all().size() > 0);
}

void CancelTest::nestedScopesRestoreOuterToken() {
    std::stop_source outer;
    const CancelScope outerScope{outer.get_token()};
    {
        std::stop_source inner;
        const CancelScope innerScope{inner.get_token()};
        inner.request_stop();
        QVERIFY(cancelRequested()); // действует внутренняя
    }
    QVERIFY(!cancelRequested()); // вернулась внешняя, её никто не отменял

    outer.request_stop();
    QVERIFY(cancelRequested());
}

QTEST_APPLESS_MAIN(CancelTest)
#include "test_cancel.moc"
