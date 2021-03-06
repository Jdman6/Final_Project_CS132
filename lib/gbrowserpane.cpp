/*
 * File: gbrowserpane.cpp
 * ----------------------
 * This file contains the implementation of the <code>GBrowserPane</code> class
 * as declared in gbrowserpane.h.
 *
 * @version 2021/04/09
 * - added sgl namespace
 * @version 2021/04/03
 * - removed dependency on non-graphical library code
 * @version 2019/04/23
 * - moved some event-handling code to GInteractor superclass
 * @version 2018/12/28
 * - added methods for text selection, scrolling, cursor position, key/mouse listeners
 * @version 2018/09/17
 * - fixed thread safety bugs
 * - added link listener events
 * @version 2018/08/23
 * - renamed to gbrowserpane.h to replace Java version
 * @version 2018/07/15
 * - initial version
 */

#include "gbrowserpane.h"
#include <fstream>
#include <QScrollBar>
#include <QTextCursor>
#include "gthread.h"
#include "require.h"
#include "privatefilelib.h"
#include "privatestrlib.h"

namespace sgl {

GBrowserPane::GBrowserPane(const std::string& url, QWidget* parent) {
    GThread::runOnQtGuiThread([this, url, parent]() {
        _iqtextbrowser = new _Internal_QTextBrowser(this, getInternalParent(parent));
    });
    if (!url.empty()) {
        readTextFromUrl(url);
    }
    setVisible(false);   // all widgets are not shown until added to a window
}

GBrowserPane::~GBrowserPane() {
    // TODO: delete _iqtextbrowser;
    _iqtextbrowser->detach();
    _iqtextbrowser = nullptr;
}

void GBrowserPane::clearSelection() {
    GThread::runOnQtGuiThread([this]() {
        QTextCursor cursor = _iqtextbrowser->textCursor();
        cursor.clearSelection();
        _iqtextbrowser->setTextCursor(cursor);
    });
}

void GBrowserPane::clearText() {
    GThread::runOnQtGuiThread([this]() {
        _iqtextbrowser->clear();
    });
}

std::string GBrowserPane::getContentType() const {
    return _contentType;
}

int GBrowserPane::getCursorPosition() const {
    return _iqtextbrowser->textCursor().position();
}

_Internal_QWidget* GBrowserPane::getInternalWidget() const {
    return _iqtextbrowser;
}

std::string GBrowserPane::getPageUrl() const {
    return _pageUrl;
}

std::string GBrowserPane::getSelectedText() const {
    QTextCursor cursor = _iqtextbrowser->textCursor();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    if (end > start) {
        return getText().substr(start, end - start);
    } else {
        return "";
    }
}

int GBrowserPane::getSelectionEnd() const {
    QTextCursor cursor = _iqtextbrowser->textCursor();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    if (end > start) {
        return end;
    } else {
        // no selection; cursor sets selection start/end to be equal
        return -1;
    }
}

int GBrowserPane::getSelectionLength() const {
    QTextCursor cursor = _iqtextbrowser->textCursor();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    return end - start;
}

int GBrowserPane::getSelectionStart() const {
    QTextCursor cursor = _iqtextbrowser->textCursor();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    if (end > start) {
        return start;
    } else {
        return -1;
    }
}

std::string GBrowserPane::getText() const {
    return _iqtextbrowser->toHtml().toStdString();
}

std::string GBrowserPane::getType() const {
    return "GBrowserPane";
}

QWidget* GBrowserPane::getWidget() const {
    return static_cast<QWidget*>(_iqtextbrowser);
}

bool GBrowserPane::isEditable() const {
    return !_iqtextbrowser->isReadOnly();
}

bool GBrowserPane::isLineWrap() const {
    return _iqtextbrowser->lineWrapMode() != QTextEdit::NoWrap;
}

void GBrowserPane::moveCursorToEnd() {
    GThread::runOnQtGuiThread([this]() {
        QTextCursor cursor = _iqtextbrowser->textCursor();
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);
        _iqtextbrowser->setTextCursor(cursor);
        _iqtextbrowser->ensureCursorVisible();
    });
}

void GBrowserPane::moveCursorToStart() {
    GThread::runOnQtGuiThread([this]() {
        QTextCursor cursor = _iqtextbrowser->textCursor();
        cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
        _iqtextbrowser->setTextCursor(cursor);
        _iqtextbrowser->ensureCursorVisible();
    });
}

void GBrowserPane::readTextFromFile(std::istream& file) {
    std::string fileText = sgl::priv::filelib::readEntireStream(file);
    setText(fileText);
}

static std::string lookupContentType(const std::string& extension) {
    static std::map<std::string, std::string> CONTENT_TYPE_MAP;   // extension => MIME type

    if (extension.empty()) {
        return "text/html";
    }

    // populate map of content types, if needed
    if (CONTENT_TYPE_MAP.empty()) {
        CONTENT_TYPE_MAP["bmp"] = "image/bmp";
        CONTENT_TYPE_MAP["bz"] = "application/x-bzip";
        CONTENT_TYPE_MAP["bz2"] = "application/x-bzip2";
        CONTENT_TYPE_MAP["c"] = "text/plain";
        CONTENT_TYPE_MAP["cc"] = "text/plain";
        CONTENT_TYPE_MAP["com"] = "application/octet-stream";
        CONTENT_TYPE_MAP["cpp"] = "text/plain";
        CONTENT_TYPE_MAP["css"] = "text/css";
        CONTENT_TYPE_MAP["doc"] = "application/msword";
        CONTENT_TYPE_MAP["dot"] = "application/msword";
        CONTENT_TYPE_MAP["exe"] = "application/octet-stream";
        CONTENT_TYPE_MAP["gif"] = "image/gif";
        CONTENT_TYPE_MAP["gz"] = "application/x-gzip";
        CONTENT_TYPE_MAP["gzip"] = "application/x-gzip";
        CONTENT_TYPE_MAP["h"] = "text/plain";
        CONTENT_TYPE_MAP["hh"] = "text/plain";
        CONTENT_TYPE_MAP["hpp"] = "text/plain";
        CONTENT_TYPE_MAP["htm"] = "text/html";
        CONTENT_TYPE_MAP["html"] = "text/html";
        CONTENT_TYPE_MAP["htmls"] = "text/html";
        CONTENT_TYPE_MAP["ico"] = "image/x-icon";
        CONTENT_TYPE_MAP["inf"] = "text/plain";
        CONTENT_TYPE_MAP["jar"] = "application/octet-stream";
        CONTENT_TYPE_MAP["jav"] = "text/plain";
        CONTENT_TYPE_MAP["java"] = "text/plain";
        CONTENT_TYPE_MAP["jpe"] = "image/jpeg";
        CONTENT_TYPE_MAP["jpeg"] = "image/jpeg";
        CONTENT_TYPE_MAP["jpg"] = "image/jpeg";
        CONTENT_TYPE_MAP["mid"] = "audio/midi";
        CONTENT_TYPE_MAP["midi"] = "audio/midi";
        CONTENT_TYPE_MAP["mod"] = "audio/mod";
        CONTENT_TYPE_MAP["mov"] = "video/quicktime";
        CONTENT_TYPE_MAP["mp3"] = "text/plain";
        CONTENT_TYPE_MAP["mpg"] = "video/mpeg";
        CONTENT_TYPE_MAP["o"] = "application/octet-stream";
        CONTENT_TYPE_MAP["odc"] = "application/vnd.oasis.opendocument.chart";
        CONTENT_TYPE_MAP["odp"] = "application/vnd.oasis.opendocument.presentation";
        CONTENT_TYPE_MAP["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
        CONTENT_TYPE_MAP["odt"] = "application/vnd.oasis.opendocument.text";
        CONTENT_TYPE_MAP["pct"] = "image/x-pict";
        CONTENT_TYPE_MAP["pcx"] = "image/x-pcx";
        CONTENT_TYPE_MAP["pdf"] = "application/pdf";
        CONTENT_TYPE_MAP["pl"] = "text/plain";
        CONTENT_TYPE_MAP["pm"] = "text/plain";
        CONTENT_TYPE_MAP["ppt"] = "application/powerpoint";
        CONTENT_TYPE_MAP["ps"] = "application/postscript";
        CONTENT_TYPE_MAP["psd"] = "application/octet-stream";
        CONTENT_TYPE_MAP["py"] = "text/plain";
        CONTENT_TYPE_MAP["qt"] = "video/quicktime";
        CONTENT_TYPE_MAP["ra"] = "audio/x-realaudio";
        CONTENT_TYPE_MAP["rb"] = "text/plain";
        CONTENT_TYPE_MAP["rm"] = "application/vnd.rn-realmedia";
        CONTENT_TYPE_MAP["rtf"] = "application/rtf";
        CONTENT_TYPE_MAP["s"] = "text/x-asm";
        CONTENT_TYPE_MAP["sh"] = "text/plain";
        CONTENT_TYPE_MAP["shtml"] = "text/html";
        CONTENT_TYPE_MAP["swf"] = "application/x-shockwave-flash";
        CONTENT_TYPE_MAP["tcl"] = "application/x-tcl";
        CONTENT_TYPE_MAP["tex"] = "application/x-tex";
        CONTENT_TYPE_MAP["tgz"] = "application/x-compressed";
        CONTENT_TYPE_MAP["tif"] = "image/tiff";
        CONTENT_TYPE_MAP["tiff"] = "image/tiff";
        CONTENT_TYPE_MAP["txt"] = "text/plain";
        CONTENT_TYPE_MAP["voc"] = "audio/voc";
        CONTENT_TYPE_MAP["wav"] = "audio/wav";
        CONTENT_TYPE_MAP["xls"] = "application/excel";
        CONTENT_TYPE_MAP["xlt"] = "application/excel";
        CONTENT_TYPE_MAP["xpm"] = "image/xpm";
        CONTENT_TYPE_MAP["z"] = "application/x-compressed";
        CONTENT_TYPE_MAP["zip"] = "application/zip";
    }

    // "foo.BAZ.BaR" => "bar"
    std::string ext = sgl::priv::strlib::toLowerCase(extension);
    int dot = sgl::priv::strlib::lastIndexOf(ext, ".");
    if (dot >= 0) {
        ext = ext.substr(dot + 1);
    }

    if (CONTENT_TYPE_MAP.find(ext) != CONTENT_TYPE_MAP.end()) {
        return CONTENT_TYPE_MAP[ext];
    } else {
        return "text/html";
    }
}

void GBrowserPane::readTextFromFile(const std::string& filename) {
    std::ifstream input;
    input.open(filename.c_str());
    if (!input.fail()) {
        _pageUrl = filename;
        std::string extension = sgl::priv::filelib::getExtension(filename);
        std::string contentType = lookupContentType(extension);
        if (!contentType.empty()) {
            setContentType(contentType);
        }
        readTextFromFile(input);
    }
}

void GBrowserPane::readTextFromUrl(const std::string& url) {
    this->_pageUrl = url;
    GThread::runOnQtGuiThread([this, url]() {
        QUrl qurl(QString::fromStdString(url));
        _iqtextbrowser->setSource(qurl);
    });
}

void GBrowserPane::removeLinkListener() {
    removeEventListener("linkclick");
}

void GBrowserPane::removeTextChangeListener() {
    removeEventListener("textchange");
}

void GBrowserPane::scrollToBottom() {
    GThread::runOnQtGuiThread([this]() {
        QScrollBar* scrollbar = _iqtextbrowser->verticalScrollBar();
        scrollbar->setValue(scrollbar->maximum());
        scrollbar->setSliderPosition(scrollbar->maximum());
    });
}

void GBrowserPane::scrollToTop() {
    GThread::runOnQtGuiThread([this]() {
        QScrollBar* scrollbar = _iqtextbrowser->verticalScrollBar();
        scrollbar->setValue(0);
        scrollbar->setSliderPosition(0);
    });
}

void GBrowserPane::select(int startIndex, int length) {
    require::nonNegative(startIndex, 0, "GBrowserPane::select", "startIndex");
    require::nonNegative(length, 0, "GBrowserPane::select", "length");
    GThread::runOnQtGuiThread([this, startIndex, length]() {
        QTextCursor cursor = _iqtextbrowser->textCursor();
        cursor.setPosition(startIndex);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, length);
        _iqtextbrowser->setTextCursor(cursor);
    });
}

void GBrowserPane::selectAll() {
    GThread::runOnQtGuiThread([this]() {
        _iqtextbrowser->selectAll();
    });
}

void GBrowserPane::setContentType(const std::string& contentType) {
    _contentType = contentType;
}

void GBrowserPane::setCursorPosition(int index, bool keepAnchor) {
    require::nonNegative(index, "TextArea::setCursorPosition", "index");
    GThread::runOnQtGuiThread([this, index, keepAnchor]() {
        QTextCursor cursor(_iqtextbrowser->textCursor());
        cursor.setPosition(index, keepAnchor ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        _iqtextbrowser->setTextCursor(cursor);
        _iqtextbrowser->ensureCursorVisible();
    });
}

void GBrowserPane::setEditable(bool value) {
    GThread::runOnQtGuiThread([this, value]() {
        _iqtextbrowser->setReadOnly(!value);
    });
}

void GBrowserPane::setMouseListener(GEventListener func) {
    setEventListeners({"mousepress",
                       "mouserelease"}, func);
}

void GBrowserPane::setMouseListener(GEventListenerVoid func) {
    setEventListeners({"mousepress",
                       "mouserelease"}, func);
}

void GBrowserPane::setLineWrap(bool wrap) {
    GThread::runOnQtGuiThread([this, wrap]() {
        _iqtextbrowser->setLineWrapMode(wrap ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
    });
}

void GBrowserPane::setLinkListener(GEventListener func) {
    setEventListener("linkclick", func);
}

void GBrowserPane::setLinkListener(GEventListenerVoid func) {
    setEventListener("linkclick", func);
}

void GBrowserPane::setText(const std::string& text) {
    GThread::runOnQtGuiThread([this, text]() {
        _iqtextbrowser->setText(QString::fromStdString(text));
    });
}

void GBrowserPane::setTextChangeListener(GEventListener func) {
    setEventListener("textchange", func);
}

void GBrowserPane::setTextChangeListener(GEventListenerVoid func) {
    setEventListener("textchange", func);
}


_Internal_QTextBrowser::_Internal_QTextBrowser(GBrowserPane* gbrowserpane, QWidget* parent)
        : QTextBrowser(parent),
          _gbrowserpane(gbrowserpane) {
    require::nonNull(gbrowserpane, "_Internal_QTextBrowser::constructor");
    setObjectName(QString::fromStdString("_Internal_QTextBrowser_" + std::to_string(gbrowserpane->getID())));
    setFocusPolicy(Qt::StrongFocus);
}

void _Internal_QTextBrowser::detach() {
    _gbrowserpane = nullptr;
}

QVariant _Internal_QTextBrowser::loadResource(int type, const QUrl &url) {
    // patch to work properly for data:... URLs
    if (type == QTextDocument::ImageResource && url.scheme() == QLatin1String("data")) {
        QRegExp regex("^image/[^;]+;base64,(.+)={0,2}$");
        if (regex.indexIn(url.path()) >= 0) {
            QImage img;
            if (img.loadFromData(QByteArray::fromBase64(regex.cap(1).toLatin1()))) {
                return QVariant::fromValue(img);
            }
        }
    }
    return QTextBrowser::loadResource(type, url);
}

void _Internal_QTextBrowser::mousePressEvent(QMouseEvent* event) {
    QTextBrowser::mousePressEvent(event);
    if (!_gbrowserpane->isAcceptingEvent("linkclick")) {
        return;
    }
    if (!(event->button() & Qt::LeftButton)) {
        return;
    }
    QString clickedAnchor = anchorAt(event->pos());
    if (clickedAnchor.isEmpty()) {
        return;
    }
    _clickedLink = clickedAnchor;
}

void _Internal_QTextBrowser::mouseReleaseEvent(QMouseEvent* event) {
    if (!_gbrowserpane->isAcceptingEvent("linkclick")) {
        QTextBrowser::mouseReleaseEvent(event);   // call super
        return;
    }
    if (!(event->button() & Qt::LeftButton)) {
        QTextBrowser::mouseReleaseEvent(event);   // call super
        return;
    }
    QString clickedAnchor = anchorAt(event->pos());
    if (clickedAnchor.isEmpty() || _clickedLink.isEmpty()
            || clickedAnchor != _clickedLink) {
        QTextBrowser::mouseReleaseEvent(event);   // call super
        return;
    }

    _clickedLink = QString::fromStdString("");   // clear

    GEvent linkEvent(
                /* class  */ HYPERLINK_EVENT,
                /* type   */ HYPERLINK_CLICKED,
                /* name   */ "linkclick",
                /* source */ _gbrowserpane);
    linkEvent.setButton(static_cast<int>(event->button()));
    linkEvent.setX(event->x());
    linkEvent.setY(event->y());
    linkEvent.setModifiers(event->modifiers());
    linkEvent.setRequestURL(clickedAnchor.toStdString());
    linkEvent.setActionCommand(_gbrowserpane->getActionCommand());
    linkEvent.setInternalEvent(event);
    _gbrowserpane->fireEvent(linkEvent);
}

QSize _Internal_QTextBrowser::sizeHint() const {
    if (hasPreferredSize()) {
        return getPreferredSize();
    } else {
        return QTextBrowser::sizeHint();
    }
}

} // namespace sgl
