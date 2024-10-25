/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
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
namespace ranges = std::ranges;
namespace rviews = std::ranges::views;

class Ruler;
class QGridLayout;
// class Scene;

template <size_t N>
struct EnumHelper2 : std::integral_constant<bool, N != 0> {
    template <typename... Es>
        requires(std::is_enum_v<Es> && ...) || (sizeof...(Es) == 0)
    constexpr EnumHelper2(Es... es)
        : array{es...} { }
    int array[N];
    template <size_t... Is>
    constexpr bool impl(auto* item, std::index_sequence<Is...>) const {
        return ((item->type() == array[Is]) || ...);
    }
    constexpr bool operator()(auto* item) const {
        return impl(item, std::make_index_sequence<N>{});
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
    //    void setScene(QGraphicsScene* Scene);
    void zoom100();
    void zoomFit();
    void zoomToSelected();
    void zoomIn();
    void zoomOut();
    void fitInView(QRectF destRect, bool withBorders = true);

    void setRuler(bool ruller);

    double scaleFactor() const noexcept { return 1.0 / getScale(); }
    QPointF mappedPos(QMouseEvent* event) const;

    void setScale(double s) noexcept;
    double getScale() const noexcept { return transform().m11(); }

    void setOpenGL(bool useOpenGL);

    void setViewRect(const QRectF& r);
    QRectF getViewRect();
    QRectF getSelectedBoundingRect();
    bool boundingRectFl() const { return boundingRect_; }

    void startUpdateTimer(int timeMs) {
        if(timerId)
            killTimer(timerId);
        // FIXME timerId = startTimer(timeMs);
    }

    void stopUpdateTimer() {
        if(timerId)
            killTimer(timerId);
        timerId = 0;
    }

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
    void mouseMove2(const QPointF&, const QPointF&);
    void mouseClickL(const QPointF&);
    void mouseClickR(const QPointF&);
    void mouseMove(const QPointF&);

private:
    template <auto Ptr, typename T, typename FilterInt, typename... Args>
    std::vector<T*> getItemImpl(FilterInt&& et, Args&&... args) const {
        const auto items = (scene()->*Ptr)(std::forward<Args>(args)...); // get all items
        constexpr bool isQGraphicsItem = std::is_same_v<T, QGraphicsItem>;
        if constexpr(isQGraphicsItem && !FilterInt::value) { //  вернуть все QGraphicsItem*
            return {items.begin(), items.end()};
        } else {
            // WARNING FilterInt faster than dynamic_cast
            // to improve speed dont use FilterDyn
            using FilterDyn = decltype([](auto* item) { return bool(dynamic_cast<T*>(item)); });
            using Transform = decltype([](auto* item) { return static_cast<T*>(item); });
            if constexpr(!isQGraphicsItem && !FilterInt::value) { // вернуть все T*
                auto rview = items | rviews::filter(FilterDyn{}) | rviews::transform(Transform{});
                return {rview.begin(), rview.end()};
            } else if constexpr(!isQGraphicsItem && FilterInt::value) { // вернуть все T* отсортированные по type()
                auto rview = items | rviews::filter(et) | rviews::filter(FilterDyn{}) | rviews::transform(Transform{});
                return {rview.begin(), rview.end()};
            } else if constexpr(isQGraphicsItem && FilterInt::value) { // вернуть все QGraphicsItem* отсортированные по type()
                auto rview = items | rviews::filter(et);
                return {rview.begin(), rview.end()};
            }
        }
    }

    Ruler* const hRuler;
    Ruler* const vRuler;
    QGridLayout* const gridLayout;
    //    Scene* scene_;
    bool ruler_{};
    int rulerCtr{};
    bool boundingRect_{};
    void updateRuler();
    template <class T>
    void animate(QObject* target, const QByteArray& propertyName, T begin, T end);
    QPoint latPos;
    QPointF point, rulPt1, rulPt2;

    void drawRuller(QPainter* painter, const QRectF& rect) const;
    int timerId{};
    void GiToShapeEvent(QMouseEvent* event, QGraphicsItem* item);
    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void drawForeground(QPainter* painter, const QRectF& rect) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;

    // QObject interface
protected:
    void timerEvent(QTimerEvent* event) override;
};

#include "app.h"
