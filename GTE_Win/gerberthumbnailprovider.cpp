#include "gerberthumbnailprovider.h"
#include <Shlwapi.h>
#include <Wincrypt.h> // For CryptStringToBinary.
#include <QtXml>
#include <QtWin>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <../G2G/clipper/myclipper.h>
#include <../G2G/gerber/gerber.h>
#include <../G2G/toolpathcreator.h>
#include "assert.h"

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "msxml6.lib")

extern HINSTANCE g_hInst;
extern long g_cDllRef;

using namespace ClipperLib;

GerberThumbnailProvider::GerberThumbnailProvider()
    : m_cRef(1)
{
    InterlockedIncrement(&g_cDllRef);
}

GerberThumbnailProvider::~GerberThumbnailProvider()
{
    InterlockedDecrement(&g_cDllRef);
}

// Query to the interface the component supported.
IFACEMETHODIMP GerberThumbnailProvider::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = {
        QITABENT(GerberThumbnailProvider, IThumbnailProvider),
        QITABENT(GerberThumbnailProvider, IInitializeWithStream),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG)
GerberThumbnailProvider::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG)
GerberThumbnailProvider::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef) {
        delete this;
    }

    return cRef;
}

// Initializes the thumbnail handler with a stream.
IFACEMETHODIMP GerberThumbnailProvider::Initialize(
    IStream* pStream,
    DWORD /*grfMode*/)
{
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
IFACEMETHODIMP GerberThumbnailProvider::GetThumbnail(
    UINT cx,
    HBITMAP* phbmp,
    WTS_ALPHATYPE* pdwAlpha)
{
    try {
        //        *phbmp = NULL;
        *pdwAlpha = WTSAT_ARGB;

        GerberParser parser;
        GERBER_FILE* gerberFile = parser.ParseLines(bytes);
        ToolPathCreator tpc;
        tpc.Merge(gerberFile);
        Pathss vpaths = tpc.GetGroupedPaths(COPPER);
        QPainterPath painterPath;
        for (Paths& paths : vpaths) {
            for (Path& path : paths) {
                painterPath.addPolygon(PathToQPolygon(path));
            }
        }
        QRectF rect = painterPath.boundingRect();
        double width, height;
        if (qFuzzyCompare(rect.width(), rect.height())) {
            width = cx;
            height = cx;
        }
        else if (rect.width() > rect.height()) {
            width = cx;
            height = rect.height() * ((double)cx / rect.width());
        }
        else {
            width = rect.width() * ((double)cx / rect.height());
            height = cx;
        }

        qreal scale = (double)cx / qMax(rect.width(), rect.height());
        QPixmap pixmap(qCeil(width), qCeil(height));
        pixmap.fill(Qt::transparent);

        QPainter painter;
        painter.begin(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::black);
        painter.setPen(Qt::NoPen);
        painter.translate(-painterPath.boundingRect().left() * scale, painterPath.boundingRect().bottom() * scale);
        painter.scale(scale, -scale);
        painter.drawPath(painterPath);
        //draw hole
        //        if (gerberFile.hole.size()) {
        //            painter.setBrush(QColor(255, 0, 0, 100));
        //            painterPath = QPainterPath();
        //            for (int i = 0; i < gerberFile.hole.size(); ++i) {
        //                painterPath.addPolygon(gerberFile.hole[i]);
        //            }
        //            painter.drawPath(painterPath);
        //        }
        painter.end();
        *pdwAlpha = WTSAT_ARGB;
        *phbmp = QtWin::toHBITMAP(pixmap, QtWin::HBitmapAlpha);
    }
    catch (std::exception& e) {
        //        QPixmap pixmap(100, 100);
        //        QPainter painter(&pixmap);
        //        pixmap.fill(Qt::transparent);
        //        painter.setRenderHint(QPainter::Antialiasing);
        //        painter.setBrush(Qt::black);
        //        painter.setPen(Qt::black);
        //        painter.drawText(QRect(0, 0, 100, 100), Qt::AlignCenter, QString("An error occurred\r\n").append(e.what()));
        //        *phbmp = QtWin::toHBITMAP(pixmap, QtWin::HBitmapAlpha);
        return S_FALSE;
    }
    catch (...) {
        return S_FALSE;
    }
    return S_OK;
}
