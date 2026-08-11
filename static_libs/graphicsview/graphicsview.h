/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QScreen>
#include <QSettings>

#include <ranges>

namespace v = std ::views;
namespace r = std ::ranges;

class Ruler;
class QGridLayout;
// class Scene;

template <size_t N>
struct EnumHelper2 : std::integral_constant<bool, N != 0> {
    template <typename... Enums>
        requires std::conjunction_v<std::is_enum<Enums>...> || (sizeof...(Enums) == 0)
    constexpr EnumHelper2(Enums... e)
        : array{e...} { }
    std::array<int, N> array;
    constexpr bool operator()(auto* item) const {
        return r::contains(array, item->type());
    }
};

template <typename... Es>
EnumHelper2(Es...) -> EnumHelper2<sizeof...(Es)>;

class GraphicsView : public QGraphicsView {
    Q_OBJECT

    Q_PROPERTY(double scale READ getScale WRITE setScale)
    Q_PROPERTY(QRectF viewRect READ getViewRect WRITE setViewRect)

public:
    explicit GraphicsView(QWidget* parent = nullptr);
    ~GraphicsView() override;
    // void setScene(QGraphicsScene* Scene);
    void zoom100();
    void zoomFit();
    void zoomToSelected();
    void zoomIn();
    void zoomOut();
    void fitInView(QRectF destRect, bool withBorders = true);

    // Показать этот участок сцены, когда виджет получит НАСТОЯЩИЙ размер.
    // Проект грузится из loadSettings() -- до show(), -- и обычный fitInView
    // считает трансформацию по начальной геометрии viewport'а, а следующий
    // resize её уже не пересчитывает: масштаб и положение остаются чужими.
    // Применяется следующим тактом цикла событий -- к тому моменту раскладка
    // досчитана и viewport имеет настоящий размер.
    void setViewRectDeferred(QRectF rect);

    void setRuler(bool ruller);

    double scaleFactor() const noexcept { return 1.0 / getScale(); }
    QPointF toScenePos(QMouseEvent* event);
    QPointF mapFromScene(const QPointF& point) const;
    QPointF mapToScene(const QPointF& point) const;
    using QGraphicsView::mapToScene;

    void setScale(double s) noexcept;
    double getScale() const noexcept { return transform().m11(); }
    void scale(double sx, double sy);

    void setOpenGL(bool useOpenGL);

    void setViewRect(const QRectF& r);
    QRectF getViewRect();
    QRectF getSelectedBoundingRect();

    // Реестр анимируемых item'ов. Раньше вид безусловно дёргал
    // scene()->update() каждые 20 мс -- полная перерисовка сцены 50 раз в
    // секунду даже на простое. Анимация нужна только «бегущим муравьям» на
    // выделенных DataPath и пульсации Gi::Error, поэтому item регистрируется
    // сам, а таймер крутится, только пока реестр непуст.
    void addAnimated(QGraphicsItem* item);
    void removeAnimated(QGraphicsItem* item);

    // Пересчитать sceneRect по фактическим данным. Звать при добавлении и
    // удалении файла, но НЕ покадрово: setSceneRect перестраивает индекс BSP
    // целиком.
    void updateSceneRectToContents();
    // То же, но с откладыванием на конец такта событий: при загрузке проекта
    // файлы добавляются по одному, а itemsBoundingRect() обходит всю сцену.
    void scheduleSceneRectUpdate();

    /////////////////////////////////
    template <typename T = QGraphicsItem, typename... Ts>
    auto items(Ts... ts) const {
        return getItemImpl<qOverload<Qt::SortOrder>(&QGraphicsScene::items), T>(EnumHelper2{ts...}, Qt::DescendingOrder);
    }

    template <typename T = QGraphicsItem, typename... Ts>
    auto selectedItems(Ts... ts) const {
        return getItemImpl<&QGraphicsScene::selectedItems, T>(EnumHelper2{ts...});
    }

    template <typename T>
    auto addItem(T* item) const { return scene()->addItem(item), item; }

    template <typename T, typename... Args>
    auto addItem(Args... args) const {
        auto item = new T{std::forward<Args>(args)...};
        return addItem(item);
    }

signals:
    void fileDroped(const QString&);
    void mouseClickL(const QPointF&);
    void mouseClickR(const QPointF&);
    void mouseMove(const QPointF&);

private:
    template <auto Ptr, typename T, typename FilterInt, typename... Args>
    std::vector<T*> getItemImpl(FilterInt&& et, Args&&... args) const {
        const auto items = (scene()->*Ptr)(std::forward<Args>(args)...); // get all items
        constexpr bool isQGraphicsItem = std::is_same_v<T, QGraphicsItem>;
        if constexpr(isQGraphicsItem && !FilterInt::value) { // вернуть все QGraphicsItem*
            return {items.begin(), items.end()};
        } else {
            // WARNING FilterInt faster than dynamic_cast
            // to improve speed dont use FilterDyn
            using FilterDyn = decltype([](auto* item) { return bool(dynamic_cast<T*>(item)); });
            using Transform = decltype([](auto* item) { return static_cast<T*>(item); });
            if constexpr(!isQGraphicsItem && !FilterInt::value) { // вернуть все T*
                auto rview = items | v::filter(FilterDyn{}) | v::transform(Transform{});
                return {rview.begin(), rview.end()};
            } else if constexpr(!isQGraphicsItem && FilterInt::value) { // вернуть все T* отсортированные по type()
                auto rview = items | v::filter(et) | v::filter(FilterDyn{}) | v::transform(Transform{});
                return {rview.begin(), rview.end()};
            } else if constexpr(isQGraphicsItem && FilterInt::value) { // вернуть все QGraphicsItem* отсортированные по type()
                auto rview = items | v::filter(et);
                return {rview.begin(), rview.end()};
            }
        }
    }

    Ruler* const hRuler;
    Ruler* const vRuler;
    QGridLayout* const gridLayout;
    // Scene* scene_;
    bool ruler_{};
    int rulerCtr{};
    void updateRuler();
    // Единая точка на смену трансформации: публикует масштаб в App и
    // подтягивает линейки.
    void onTransformChanged();
    template <class T>
    void animate(QObject* target, const QByteArray& propertyName, T begin, T end);
    QPointF scenePos, rulPt1, rulPt2, pressPos;

    // Экранные украшения (перекрестье под курсором и мерная линейка) рисуются
    // поверх вьюпорта в его собственных координатах, а не в слое сцены: плечи
    // у них заданы в пикселях, и сцене незачем перерисовываться ради них.
    void drawOverlay(QPainter* painter);
    void drawRuller(QPainter* painter) const;
    QRegion overlayRegion() const;
    // Перерисовать только полосы перекрестья (старые и новые), не весь вид.
    void updateOverlay();
    QPoint cursorViewPos_;

    std::vector<QGraphicsItem*> animated_;
    int animTimerId_{};
    void updateAnimationTimer();

    bool sceneRectUpdatePending_{};

    void GiToShapeEvent(QMouseEvent* event, QGraphicsItem* item);
    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void scrollContentsBy(int dx, int dy) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    // Фон рисует сам QGraphicsView кистью из setBackgroundBrush.
    void drawForeground(QPainter* painter, const QRectF& rect) override;

    // QObject interface
protected:
    void timerEvent(QTimerEvent* event) override;

private:
    // Отложенный вид: пуст, пока восстанавливать нечего.
    QRectF pendingViewRect_;
    void applyPendingViewRect();
};

#include "app.h"
