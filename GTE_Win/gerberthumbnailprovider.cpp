// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
 * Author    :  Bakiev Damir                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Bakiev Damir 2010-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/

#include "gerberthumbnailprovider.h"
#include "assert.h"
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QtWin>
#include <Shlwapi.h>
#include <clipper/myclipper.h>
#include <gerber/gbr_parser.h>
#include <wincrypt.h> // For CryptStringToBinary.

#include <signal.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "msxml6.lib")

extern HINSTANCE g_hInst;
extern long g_cDllRef;

using namespace ClipperLib;

GerberThumbnailProvider::GerberThumbnailProvider()
    : m_cRef(1) {
    InterlockedIncrement(&g_cDllRef);
}

GerberThumbnailProvider::~GerberThumbnailProvider() {
    InterlockedDecrement(&g_cDllRef);
}

// Query to the interface the component supported.
IFACEMETHODIMP GerberThumbnailProvider::QueryInterface(REFIID riid, void** ppv) {
    static const QITAB qit[] = {
        QITABENT(GerberThumbnailProvider, IThumbnailProvider),
        QITABENT(GerberThumbnailProvider, IInitializeWithStream),
        { nullptr, 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG)
GerberThumbnailProvider::AddRef() {
    return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG)
GerberThumbnailProvider::Release() {
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef) {
        delete this;
    }

    return cRef;
}

// Initializes the thumbnail handler with a stream.
IFACEMETHODIMP GerberThumbnailProvider::Initialize(
    IStream* pStream,
    DWORD /*grfMode*/) {
    //    // A handler instance should be initialized only once in its lifetime.
    //    HRESULT hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    //    if (m_pStream == NULL) {
    //        // Take a reference to the stream if it has not been initialized yet.
    //        hr = pStream->QueryInterface(&m_pStream);
    //    }

    ULONG len;
    STATSTG stat;
    if (pStream->Stat(&stat, STATFLAG_DEFAULT) != S_OK) {
        return S_FALSE;
    }

    char* data = new char[stat.cbSize.QuadPart];

    if (pStream->Read(data, stat.cbSize.QuadPart, &len) != S_OK) {
        return S_FALSE;
    }

    bytes = QByteArray(data, stat.cbSize.QuadPart);

    delete[] data;

    return S_OK;
}

// Gets a thumbnail image and alpha type. The GetThumbnail is called with the
// largest desired size of the image, in pixels. Although the parameter is
// called cx, this is used as the maximum size of both the x and y dimensions.
// If the retrieved thumbnail is not square, then the longer axis is limited
// by cx and the aspect ratio of the original image respected. On exit,
// GetThumbnail provides a handle to the retrieved image. It also provides a
// value that indicates the color format of the image and whether it has
// valid alpha information.

void SignalHandler(int signal) {
    printf("Signal %d", signal);
    throw "!Access Violation!";
}
IFACEMETHODIMP GerberThumbnailProvider::GetThumbnail(
    UINT cx,
    HBITMAP* phbmp,
    WTS_ALPHATYPE* pdwAlpha) {

    typedef void (*SignalHandlerPointer)(int);
    SignalHandlerPointer previousHandler;
    previousHandler = signal(SIGSEGV, SignalHandler);
    try {
        *pdwAlpha = WTSAT_ARGB;

        QPainterPath painterPath;
        Paths paths = Gerber::Parser().parseLines(bytes);
        if (paths.isEmpty())
            throw std::exception("Gerber::Parser().parseLines(bytes) error");
        for (Path& path : paths) {
            if (path.size() > 2) {
                path.append(path.first());
                painterPath.addPolygon(toQPolygon(path));
            }
        }
        QRectF rect = painterPath.boundingRect();
        double width, height;
        if (qFuzzyCompare(rect.width(), rect.height())) {
            width = cx;
            height = cx;
        } else if (rect.width() > rect.height()) {
            width = cx;
            height = rect.height() * ((double)cx / rect.width());
        } else {
            width = rect.width() * ((double)cx / rect.height());
            height = cx;
        }

        qreal scale = (double)cx / qMax(rect.width(), rect.height());
        QPixmap pixmap(qCeil(width), qCeil(height));
        pixmap.fill(Qt::/*white*/ transparent);

        QPainter painter;
        painter.begin(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::black);
        painter.setPen(Qt::NoPen); // QPen(Qt::black, 0.0));
        painter.translate(-painterPath.boundingRect().left() * scale, painterPath.boundingRect().bottom() * scale);
        painter.scale(scale, -scale);
        painter.drawPath(painterPath);
        // draw hole
        //         if (gerberFile.hole.size()) {
        //             painter.setBrush(QColor(255, 0, 0, 100));
        //             painterPath = QPainterPath();
        //             for (int i = 0; i < gerberFile.hole.size(); ++i) {
        //                 painterPath.addPolygon(gerberFile.hole[i]);
        //             }
        //             painter.drawPath(painterPath);
        //         }
        painter.end();
        *pdwAlpha = WTSAT_ARGB;
        *phbmp = QtWin::toHBITMAP(pixmap, QtWin::HBitmapAlpha);
    } catch (const std::exception& e) {
        QPixmap pixmap(static_cast<int>(cx), static_cast<int>(cx));
        QPainter painter(&pixmap);
        pixmap.fill(Qt::white);
        QFont f;
        f.setPixelSize(cx / 20);
        painter.setFont(f);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::red);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, QString("An error occurred\r\n").append(e.what()));
        *phbmp = QtWin::toHBITMAP(pixmap, QtWin::HBitmapAlpha);
        signal(SIGSEGV, previousHandler);
        return S_FALSE;
    } catch (char* e) {
        QPixmap pixmap(static_cast<int>(cx), static_cast<int>(cx));
        QPainter painter(&pixmap);
        pixmap.fill(Qt::white);
        QFont f;
        f.setPixelSize(cx / 20);
        painter.setFont(f);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::red);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, e);
        *phbmp = QtWin::toHBITMAP(pixmap, QtWin::HBitmapAlpha);
        signal(SIGSEGV, previousHandler);
        return S_FALSE;
    } catch (...) {
        QPixmap pixmap(static_cast<int>(cx), static_cast<int>(cx));
        QPainter painter(&pixmap);
        pixmap.fill(Qt::white);
        QFont f;
        f.setPixelSize(cx / 20);
        painter.setFont(f);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::red);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, "An error occurred");
        *phbmp = QtWin::toHBITMAP(pixmap, QtWin::HBitmapAlpha);
        signal(SIGSEGV, previousHandler);
        return S_FALSE;
    }
    signal(SIGSEGV, previousHandler);
    return S_OK;
}
