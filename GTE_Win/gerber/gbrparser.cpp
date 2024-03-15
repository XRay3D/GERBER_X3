// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2020                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gbr_parser.h"
#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QTextStream>
#include <QThread>

namespace Gerber {

Parser::Parser(QObject* parent)
    : QObject(parent) {
}

Paths Parser::parseFile(const QString& fileName) {
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly | QFile::Text))
        return Paths();
    QTextStream in(&file);
    return parseLines(in.readAll());
}

Paths Parser::parseLines(const QString& gerberLines) {
    try {
        QElapsedTimer t;
        t.start();

        static const QRegExp match(QStringLiteral("FS([L|T]?)([A|I]?)X(\\d)(\\d)Y(\\d)(\\d)\\*"));
        if(match.indexIn(gerberLines) == -1) {
            // emit fileError("", QFileInfo(fileName).fileName() + "\n" + "Incorrect File!\nNot contains format.");
            return Paths();
        }

        reset();
        //    m_file->read();
        //    if (m_file->size()) {
        //        //emit fileProgress(m_file->shortName(),m_lines.size(), 0);
        //        m_file->createGi();
        //        //emit fileReady(m_file);
        //        //emit fileProgress(m_file->shortName(), 1, 1);
        //        mutex.unlock();

        //        return;
        //    }

        m_lines = format(gerberLines);
        if(m_lines.isEmpty())
            return Paths();
        qDebug() << m_lines;
        m_lineNum = 0;

        for(const QString& gerberLine: m_lines) {
            m_currentGerbLine = gerberLine;
            ++m_lineNum;

            if(parseGCode(gerberLine))
                continue;

            if(parseDCode(gerberLine))
                continue;

            if(parseStepRepeat(gerberLine))
                continue; //////////

            if(parseApertureBlock(gerberLine))
                continue;

            if(parseAttributes(gerberLine))
                continue;

            if(parseImagePolarity(gerberLine))
                continue;

            if(parseApertureMacros(gerberLine))
                continue;

            // Aperture definitions %ADD...
            if(parseAperture(gerberLine))
                continue;

            // Polarity change
            // Example: %LPD*% or %LPC*%
            // If polarity changes, creates geometry from current
            // buffer, then adds or subtracts accordingly.
            if(parseTransformations(gerberLine))
                continue;

            // Number format
            // Example: %FSLAX24Y24*%
            // TODO: This is ignoring most of the format-> Implement the rest.
            if(parseFormat(gerberLine))
                continue;

            // Mode (IN/MM)
            // Example: %MOIN*%
            if(parseUnitMode(gerberLine))
                continue;

            if(parseEndOfFile(gerberLine))
                continue;

            // G01 - Linear interpolation plus flashes
            // Operation code (D0x) missing is deprecated... oh well I will support it.
            if(parseLineInterpolation(gerberLine))
                continue;

            // G02/G03 - Circular interpolation
            // 2-clockwise, 3-counterclockwise
            if(parseCircularInterpolation(gerberLine))
                continue;

            // Line didn`t match any pattern. Warn user.
            qWarning() << QString("Line ignored (%1): '%2'").arg(m_lineNum).arg(gerberLine);
        }
        if(m_file.isEmpty()) {

        } else {
            return merge();
            /*mergedPaths();*/
            // m_file->itemGroup()->setVisible(true);
            // emit fileReady(m_file);
        }
        // emit fileProgress(m_file->shortName(), 1, 1);
        m_currentGerbLine.clear();
        m_apertureMacro.clear();
        m_path.clear();
    } catch(const QString& errStr) {
        qWarning() << "exeption Q:" << errStr;
    } catch(const char* errStr) {
        qWarning() << "exeption Q:" << errStr;
    } catch(...) {
        qWarning() << "exeption S:" << strerror(errno) << errno;
    }
    return Paths();
}

QList<QString> Parser::format(QString data) {
    QList<QString> gerberLines;

    enum State {
        Param,
        Macro,
        Data,
    };

    State state = Data;
    QString lastLine;

    auto gerberLinesAppend = [&gerberLines, &lastLine](State& state, const QString& val) -> void {
        switch(state) {
        case Macro:
            lastLine.push_back(val);
            if(lastLine.endsWith('%')) {
                gerberLines << lastLine;
                state = Data;
            }
            break;
        case Param:
            lastLine.push_back(val);
            if(lastLine.endsWith('%')) {
                for(QString& tmpline: lastLine.remove('%').split('*'))
                    if(!tmpline.isEmpty())
                        gerberLines << ('%' + tmpline + "*%");
                state = Data;
            }
            break;
        case Data:
            break;
        }
    };

    auto lastLineClose = [&gerberLines](State state, QString& val) -> void {
        switch(state) {
        case Macro:
            if(!val.endsWith('%'))
                val.push_back('%');
            if(!val.endsWith("*%"))
                val.insert(val.length() - 2, '*');
            gerberLines << val;
            break;
        case Param:
            for(QString& tmpline: val.remove('%').split('*'))
                if(!tmpline.isEmpty())
                    gerberLines << ('%' + tmpline + "*%");
            break;
        case Data:
            break;
        }
        val.clear();
    };

    auto dataClose = [&gerberLines](const QString& val) -> void {
        if(val.count('*') > 1) {
            for(QString& tmpline: val.split('*'))
                if(!tmpline.isEmpty())
                    gerberLines << (tmpline + '*');
        } else {
            gerberLines << val;
        }
    };
    for(QString& line: data.replace('\r', '\n').replace("\n\n", "\n").replace('\t', ' ').split('\n')) {
        line = line.trimmed();

        if(line.isEmpty())
            continue;
        if(line == '*')
            continue;

        if(line.startsWith('%') && line.endsWith('%') && line.size() > 1) {
            lastLineClose(state, lastLine);
            if(line.startsWith("%AM"))
                lastLineClose(Macro, line);
            else
                lastLineClose(Param, line);
            state = Data;
            continue;
        } else if(line.startsWith("%AM")) {
            lastLineClose(state, lastLine);
            state = Macro;
            lastLine = line;
            continue;
        } else if(line.startsWith('%')) {
            lastLineClose(state, lastLine);
            state = Param;
            lastLine = line;
            continue;
        } else if(line.endsWith('*') && line.length() > 1) {
            switch(state) {
            case Macro:
            case Param:
                gerberLinesAppend(state, line);
                continue;
            case Data:
                dataClose(line);
                continue;
            }
        } else {
            switch(state) {
            case Macro:
            case Param:
                gerberLinesAppend(state, line);
                continue;
            case Data:
                // qWarning() << "Хрен его знает:" << line;
                continue;
            }
        }
    }
    return gerberLines;
}

double Parser::arcAngle(double start, double stop) {
    if(m_state.interpolation() == CounterclockwiseCircular && stop <= start)
        stop += 2.0 * pi;
    if(m_state.interpolation() == ClockwiseCircular && stop >= start)
        stop -= 2.0 * pi;
    return qAbs(stop - start);
}

double Parser::toDouble(const QString& Str, bool scale, bool inchControl) {
    bool ok;
    double d = Str.toDouble(&ok);
    if(m_state.format()->unitMode == Inches && inchControl)
        d *= 25.4;
    if(scale)
        d *= uScale;
    return d;
}

bool Parser::parseNumber(QString Str, cInt& val, int integer, int decimal) {
    bool flag = false;
    int sign = 1;
    if(!Str.isEmpty()) {
        if(!decimal)
            decimal = m_state.format()->xDecimal;

        if(!integer)
            integer = m_state.format()->xInteger;

        if(Str.indexOf("+") == 0) {
            Str.remove(0, 1);
            sign = 1;
        }

        if(Str.indexOf("-") == 0) {
            Str.remove(0, 1);
            sign = -1;
        }

        if(Str.count('.'))
            Str.setNum(Str.split('.').first().toInt() + ("0." + Str.split('.').last()).toDouble());

        while(Str.length() < integer + decimal) {
            switch(m_state.format()->zeroOmisMode) {
            case OmitLeadingZeros:
                Str = QString(integer + decimal - Str.length(), '0') + Str;
                // Str = "0" + Str;
                break;
#ifdef DEPRECATED
            case OmitTrailingZeros:
                Str += QString(integer + decimal - Str.length(), '0');
                // Str += "0";
                break;
#endif
            }
        }
        val = static_cast<cInt>(toDouble(Str, true) * pow(10.0, -decimal) * sign);
        return true;
    }
    return flag;
}

void Parser::addPath() {
    if(m_path.size() < 2) {
        resetStep();
        return;
    }
    switch(m_state.region()) {
    case On:
        m_state.setType(Region);
        switch(m_abSrIdStack.top().first) {
        case Normal:
            m_file.append(GraphicObject(m_goId++, m_state, createPolygon(), nullptr, m_path));
            break;
        case StepRepeat:
            m_stepRepeat.storage.append(GraphicObject(m_goId++, m_state, createPolygon(), nullptr, m_path));
            break;
        case ApertureBlock:
            apBlock(m_abSrIdStack.top().second)->append(GraphicObject(m_goId++, m_state, createPolygon(), nullptr, m_path));
            break;
        }
        break;
    case Off:
        m_state.setType(Line);
        switch(m_abSrIdStack.top().first) {
        case Normal:
            m_file.append(GraphicObject(m_goId++, m_state, createLine(), nullptr, m_path));
            break;
        case StepRepeat:
            m_stepRepeat.storage.append(GraphicObject(m_stepRepeat.storage.size(), m_state, createLine(), nullptr, m_path));
            break;
        case ApertureBlock:
            apBlock(m_abSrIdStack.top().second)->append(GraphicObject(apBlock(m_abSrIdStack.top().second)->size(), m_state, createLine(), nullptr, m_path));
            break;
        }
        break;
    }
    resetStep();
}

void Parser::addFlash() {
    m_state.setType(Aperture);
    if(!m_apertures.contains(m_state.aperture()) && m_apertures[m_state.aperture()].data() == nullptr)
        throw tr("Aperture %1 not found!").arg(m_state.aperture());

    AbstractAperture* ap = m_apertures[m_state.aperture()].data();
    Paths paths(ap->draw(m_state, m_abSrIdStack.top().first != ApertureBlock));
    ////////////////////////////////// Draw Drill //////////////////////////////////
    if(ap->isDrilled())
        paths.push_back(ap->drawDrill(m_state));

    switch(m_abSrIdStack.top().first) {
    case Normal:
        m_file.append(GraphicObject(m_goId++, m_state, paths, nullptr));
        break;
    case StepRepeat:
        m_stepRepeat.storage.append(GraphicObject(m_stepRepeat.storage.size(), m_state, paths, nullptr));
        break;
    case ApertureBlock:
        apBlock(m_abSrIdStack.top().second)->append(GraphicObject(apBlock(m_abSrIdStack.top().second)->size(), m_state, paths, nullptr));
        break;
    }
    resetStep();
}

void Parser::reset() {
    m_currentGerbLine.clear();
    m_apertureMacro.clear();
    m_path.clear();
    m_file.clear();
    m_state = State(&m_format);
    m_abSrIdStack.clear();
    m_abSrIdStack.push({Normal, 0});
    m_stepRepeat.reset();
    m_goId = 0;
}

void Parser::resetStep() {
    m_currentGerbLine.clear();
    m_path.clear();
    m_path.push_back(m_state.curPos());
}

IntPoint Parser::parsePosition(const QString& xyStr) {
    static const QRegExp match(QStringLiteral("(?:G[01]{1,2})?(?:X([+-]?\\d*\\.?\\d+))?(?:Y([+-]?\\d*\\.?\\d+))?"));
    if(match.indexIn(xyStr) > -1) {
        cInt tmp = 0;
        if(parseNumber(match.cap(1), tmp, m_state.format()->xInteger, m_state.format()->xDecimal))
            m_state.format()->coordValueNotation == AbsoluteNotation ? m_state.curPos().X = tmp : m_state.curPos().X += tmp;
        tmp = 0;
        if(parseNumber(match.cap(2), tmp, m_state.format()->yInteger, m_state.format()->yDecimal))
            m_state.format()->coordValueNotation == AbsoluteNotation ? m_state.curPos().Y = tmp : m_state.curPos().Y += tmp;
    }
    if(2.0e-310 > m_state.curPos().X && m_state.curPos().X > 0.0)
        throw tr("line num %1: '%2', error value.").arg(m_lineNum).arg(m_currentGerbLine);
    if(2.0e-310 > m_state.curPos().Y && m_state.curPos().Y > 0.0)
        throw tr("line num %1: '%2', error value.").arg(m_lineNum).arg(m_currentGerbLine);
    return m_state.curPos();
}

Path Parser::arc(const IntPoint& center, double radius, double start, double stop) {
    const double da_sign[4] = {0, 0, -1.0, +1.0};
    Path points;

    const int intSteps = 18; // MinStepsPerCircle;

    if(m_state.interpolation() == ClockwiseCircular && stop >= start)
        stop -= 2.0 * pi;
    else if(m_state.interpolation() == CounterclockwiseCircular && stop <= start)
        stop += 2.0 * pi;

    double angle = qAbs(stop - start);
    double steps = qMax(static_cast<int>(ceil(angle / (2.0 * pi) * intSteps)), 2);
    double delta_angle = da_sign[m_state.interpolation()] * angle * 1.0 / steps;
    for(int i = 0; i < steps; i++) {
        double theta = start + delta_angle * (i + 1);
        points.push_back(IntPoint(
            static_cast<cInt>(center.X + radius * cos(theta)),
            static_cast<cInt>(center.Y + radius * sin(theta))));
    }

    return points;
}

Path Parser::arc(IntPoint p1, IntPoint p2, IntPoint center) {
    double radius = sqrt(pow((center.X - p1.X), 2) + pow((center.Y - p1.Y), 2));
    double start = atan2(p1.Y - center.Y, p1.X - center.X);
    double stop = atan2(p2.Y - center.Y, p2.X - center.X);
    return arc(center, radius, start, stop);
}

Paths Parser::createLine() {
    Paths solution;
    if(!m_apertures.contains(m_state.aperture()))
        throw tr("Aperture %1 not found!").arg(m_state.aperture());

    if(m_apertures[m_state.aperture()]->type() == Rectangle) {
        ApRectangle* rect = static_cast<ApRectangle*>(m_apertures[m_state.aperture()].data());
        if(!qFuzzyCompare(rect->m_width, rect->m_height)) // only square Aperture
            throw tr("Aperture D%1 (%2) not supported!").arg(m_state.aperture()).arg(rect->name());
        double size = rect->m_width * uScale * 0.5 * m_state.scaling();
        if(qFuzzyIsNull(size))
            size = 1;
        ClipperOffset offset;
        offset.AddPath(m_path, jtSquare, etOpenSquare);
        offset.Execute(solution, size);
        if(m_state.imgPolarity() == Negative)
            ReversePaths(solution);
    } else {
        double size = m_apertures[m_state.aperture()]->apertureSize() * uScale * 0.5 * m_state.scaling();
        if(qFuzzyIsNull(size))
            size = 1;
        ClipperOffset offset;
        offset.AddPath(m_path, jtRound, etOpenRound);
        offset.Execute(solution, size);
        if(m_state.imgPolarity() == Negative)
            ReversePaths(solution);
    }
    return solution;
}

Paths Parser::createPolygon() {
    if(Area(m_path) > 0.0) {
        if(m_state.imgPolarity() == Negative)
            ReversePath(m_path);
    } else {
        if(m_state.imgPolarity() == Positive)
            ReversePath(m_path);
    }
    return {m_path};
}

bool Parser::parseAperture(const QString& gLine) {
    static const QRegExp match(QStringLiteral("^%ADD(\\d\\d+)([a-zA-Z_$\\.][a-zA-Z0-9_$\\.\\-]*)(?:,(.*))?\\*%$"));
    static const QList<QString> slApertureType({"C", "R", "O", "P", "M"});
    if(match.exactMatch(gLine)) {
        int aperture = /*state().aperture =*/match.cap(1).toInt();
        QString apType = match.cap(2);
        QString apParameters = match.cap(3);

        // Parse gerber aperture definition into dictionary of apertures.
        // The following kinds and their attributes are supported:
        // * Circular (C)*: size (float)
        // * Rectangle (R)*: width (float), height (float)
        // * Obround (O)*: width (float), height (float).
        // * Polygon (P)*: diameter(float), vertices(int), [rotation(float)]
        // * Aperture Macro (AM)*: macro (ApertureMacro), modifiers (list)

        QList<QString> paramList = apParameters.split("X");
        double hole = 0.0, rotation = 0.0;
        switch(slApertureType.indexOf(apType)) {
        case Circle:
            if(paramList.size() > 1)
                hole = toDouble(paramList[1]);
            m_apertures[aperture] = QSharedPointer<AbstractAperture>(new ApCircle(toDouble(paramList[0]), hole, &m_format));
            break;
        case Rectangle:
            if(paramList.size() > 2)
                hole = toDouble(paramList[2]);
            m_apertures.insert(aperture, QSharedPointer<AbstractAperture>(new ApRectangle(toDouble(paramList[0]), toDouble(paramList[1]), hole, &m_format)));
            break;
        case Obround:
            if(paramList.size() > 2)
                hole = toDouble(paramList[2]);
            m_apertures.insert(aperture, QSharedPointer<AbstractAperture>(new ApObround(toDouble(paramList[0]), toDouble(paramList[1]), hole, &m_format)));
            break;
        case Polygon:
            if(paramList.length() > 2)
                rotation = toDouble(paramList[2], false, false);
            if(paramList.length() > 3)
                hole = toDouble(paramList[3]);
            m_apertures.insert(aperture, QSharedPointer<AbstractAperture>(new ApPolygon(toDouble(paramList[0]), paramList[1].toInt(), rotation, hole, &m_format)));
            break;
        case Macro:
        default:
            QMap<QString, double> macroCoeff;
            for(int i = 0; i < paramList.size(); ++i)
                macroCoeff[QString("$%1").arg(i + 1)] = toDouble(paramList[i], false, false);
            m_apertures.insert(aperture, QSharedPointer<AbstractAperture>(new ApMacro(apType, m_apertureMacro[apType].split('*'), macroCoeff, &m_format)));
            break;
        }
        return true;
    }
    return false;
}

bool Parser::parseApertureBlock(const QString& gLine) {
    static const QRegExp match(QStringLiteral("^%ABD(\\d+)\\*%$"));
    if(match.exactMatch(gLine)) {
        m_abSrIdStack.push({ApertureBlock, match.cap(1).toInt()});
        m_apertures.insert(m_abSrIdStack.top().second, QSharedPointer<AbstractAperture>(new ApBlock(&m_format)));
        return true;
    }
    if(gLine == "%AB*%") {
        addPath();
        m_abSrIdStack.pop();
        return true;
    }
    return false;
}

bool Parser::parseTransformations(const QString& gLine) {
    enum {
        trPolarity,
        trMirror,
        trRotate,
        trScale,
    };
    static const QStringList slTransformations{"P", "M", "R", "S"};
    static const QStringList slLevelPolarity{"D", "C"};
    static const QStringList slLoadMirroring{"N", "X", "Y", "XY"};
    static const QRegExp match(QStringLiteral("^%L([PMRS])(.+)\\*%$"));
    if(match.exactMatch(gLine)) {
        switch(slTransformations.indexOf(match.cap(1))) {
        case trPolarity:
            addPath();
            switch(slLevelPolarity.indexOf(match.cap(2))) {
            case Positive:
                m_state.setImgPolarity(Positive);
                break;
            case Negative:
                m_state.setImgPolarity(Negative);
                break;
            default:
                throw "Polarity error!";
            }
            return true;
        case trMirror:
            m_state.setMirroring(static_cast<Mirroring>(slLoadMirroring.indexOf(match.cap(2))));
            return true;
        case trRotate:
            m_state.setRotating(match.cap(2).toDouble());
            return true;
        case trScale:
            m_state.setScaling(match.cap(2).toDouble());
            return true;
        }
    }
    return false;
}

bool Parser::parseStepRepeat(const QString& gLine) {
    //    return false;
    //<SR open>      = %SRX<Repeats>Y<Repeats>I<Step>J<Step>*%
    //<SR close>     = %SR*%
    //<SR statement> = <SR open>{<single command>|<region statement>}<SR close>
    static const QRegExp match(QStringLiteral("^%SRX(\\d+)Y(\\d+)I([+-]?\\d*\\.?\\d*)J([+-]?\\d*\\.?\\d*)\\*%$"));
    if(match.exactMatch(gLine)) {
        if(m_abSrIdStack.top().first == StepRepeat)
            closeStepRepeat();
        m_stepRepeat.reset();
        m_stepRepeat.x = match.cap(1).toInt();
        m_stepRepeat.y = match.cap(2).toInt();
        m_stepRepeat.i = match.cap(3).toDouble() * uScale;
        m_stepRepeat.j = match.cap(4).toDouble() * uScale;
        if(m_state.format()->unitMode == Inches) {
            m_stepRepeat.i *= 25.4;
            m_stepRepeat.j *= 25.4;
        }
        if(m_stepRepeat.x > 1 || m_stepRepeat.y > 1)
            m_abSrIdStack.push({StepRepeat, 0});
        return true;
    }
    QRegExp match2("^%SR\\*%$");
    if(match2.exactMatch(gLine)) {
        if(m_abSrIdStack.top().first == StepRepeat)
            closeStepRepeat();
        return true;
    }
    return false;
}

void Parser::closeStepRepeat() {
    addPath();
    for(int y = 0; y < m_stepRepeat.y; ++y) {
        for(int x = 0; x < m_stepRepeat.x; ++x) {
            const IntPoint pt(static_cast<cInt>(m_stepRepeat.i * x), static_cast<cInt>(m_stepRepeat.j * y));
            for(GraphicObject& go: m_stepRepeat.storage) {
                Paths paths(go.paths());
                for(Path& path: paths)
                    TranslatePath(path, pt);
                Path path(go.path());
                TranslatePath(path, pt);
                go.state().setCurPos({go.state().curPos().X + pt.X, go.state().curPos().Y + pt.Y});
                m_file.append(GraphicObject(m_goId++, go.state(), paths, nullptr, path));
            }
        }
    }
    m_stepRepeat.reset();
    m_abSrIdStack.pop();
}

bool Parser::parseApertureMacros(const QString& gLine) {
    static const QRegExp match(QStringLiteral("^%AM([^\\*]+)\\*([^%]+)?(%)?$"));
    // Start macro if(match, else not an AM, carry on.
    if(match.exactMatch(gLine)) {
        if(!match.cap(2).isEmpty() && !match.cap(3).isEmpty()) {
            m_apertureMacro[match.cap(1)] = match.cap(2);
            return true;
        }
    }
    return false;
}

bool Parser::parseAttributes(const QString& gLine) {
    static const QRegExp match(QStringLiteral("^%T(F|A|O|D)(.*)\\*%$"));
    if(match.exactMatch(gLine)) {
        // const QList<QString> slAttributeType(QString("TF|TA|TO|TD").split("|"));
        //  switch (slAttributeType.indexOf(match.cap(1))) {
        //  case ATTRIBUTE:
        //  //FileFunction
        //  gerberFile.attributesStrings.push_back(match.cap(2));
        //  break;
        //  case APERTURE_ATTRIBUTE:
        //  gerberFile.apertureAttributesStrings.push_back(match.cap(2));
        //  break;
        //  case OBJECT_ATTRIBUTE:
        //  gerberFile.objectAttributesStrings.push_back(match.cap(2));
        //  break;
        //  case DELETE_ATTRIBUTE:
        //  for (int i = 0; i < gerberFile.attributesStrings.size(); ++i) {
        //  if (gerberFile.attributesStrings[i].indexOf(match.cap(1)) >= 0) {
        //  gerberFile.attributesStrings.removeAt(i);
        //  }
        //  }
        //  for (int i = 0; i < gerberFile.apertureAttributesStrings.size(); ++i) {
        //  if (gerberFile.apertureAttributesStrings[i].indexOf(match.cap(1)) >= 0) {
        //  gerberFile.apertureAttributesStrings.removeAt(i);
        //  }
        //  }
        //  for (int i = 0; i < gerberFile.objectAttributesStrings.size(); ++i) {
        //  if (gerberFile.objectAttributesStrings[i].indexOf(match.cap(1)) >= 0) {
        //  gerberFile.objectAttributesStrings.removeAt(i);
        //  }
        //  }
        //  break;
        //  }
        return true;
    }
    return false;
}

bool Parser::parseCircularInterpolation(const QString& gLine) {
    static const QRegExp match(QStringLiteral("^(?:G0?([23]))?[X]?([\\+-]?\\d+)*[Y]?([\\+-]?\\d+)*[I]?([\\+-]?\\d+)*[J]?([\\+-]?\\d+)*[^D]*(?:D0?([12]))?\\*$"));

    if(match.exactMatch(gLine)) {
        if(match.cap(1).isEmpty() && m_state.gCode() != G02 && m_state.gCode() != G03)
            return false;

        cInt x = 0, y = 0, i = 0, j = 0;
        if(match.cap(2).isEmpty())
            x = m_state.curPos().X;
        else
            parseNumber(match.cap(2), x, m_state.format()->xInteger, m_state.format()->xDecimal);

        if(match.cap(3).isEmpty())
            y = m_state.curPos().Y;
        else
            parseNumber(match.cap(3), y, m_state.format()->yInteger, m_state.format()->yDecimal);

        parseNumber(match.cap(4), i, m_state.format()->xInteger, m_state.format()->xDecimal);
        parseNumber(match.cap(5), j, m_state.format()->yInteger, m_state.format()->yDecimal);

        switch(match.cap(1).toInt()) {
        case G02:
            m_state.setInterpolation(ClockwiseCircular);
            m_state.setGCode(G02);
            break;
        case G03:
            m_state.setInterpolation(CounterclockwiseCircular);
            m_state.setGCode(G03);
            break;
        default:
            if(m_state.interpolation() != ClockwiseCircular && m_state.interpolation() != CounterclockwiseCircular) {
                qWarning() << QString("Found arc without circular interpolation mode defined. (%1)").arg(m_lineNum);
                qWarning() << QString(gLine);
                m_state.setCurPos({x, y});
                m_state.setGCode(G01);
                return false;
            }
            break;
        }

        if(m_state.quadrant() == Undef) {
            qWarning() << QString("Found arc without preceding quadrant specification G74 or G75. (%1)").arg(m_lineNum);
            qWarning() << QString(gLine);
            return true;
        }

        // Set operation code if provided
        if(!match.cap(6).isEmpty())
            m_state.setDCode(static_cast<DCode>(match.cap(6).toInt()));
        switch(m_state.dCode()) {
        case D01:
            break;
        case D02: // Nothing created! Pen Up.
            m_state.setDCode(D01);
            addPath();
            m_state.setCurPos({x, y});
            return true;
        case D03: // Flash should not happen here
            m_state.setCurPos({x, y});
            qWarning() << QString("Trying to flash within arc. (%1)").arg(m_lineNum);
            return true;
        }

        const IntPoint& curPos = m_state.curPos();

        const IntPoint centerPos[4] = {
            {curPos.X + i, curPos.Y + j},
            {curPos.X - i, curPos.Y + j},
            {curPos.X + i, curPos.Y - j},
            {curPos.X - i, curPos.Y - j}
        };

        bool valid = false;

        m_path.push_back(m_state.curPos());
        Path arcPolygon;
        switch(m_state.quadrant()) {
        case Multi: // G75
        {
            const double radius1 = sqrt(pow(i, 2.0) + pow(j, 2.0));
            const double start = atan2(-j, -i); // Start angle
            // Численные ошибки могут помешать, start == stop, поэтому мы проверяем заблаговременно.
            // Ч­то должно привести к образованию дуги в 360 градусов.
            const double stop = (m_state.curPos() == IntPoint(x, y)) ? start : atan2(-centerPos[0].Y + y, -centerPos[0].X + x); // Stop angle

            arcPolygon = arc(IntPoint(centerPos[0].X, centerPos[0].Y), radius1, start, stop);
            // arcPolygon = arc(curPos, IntPoint(x, y), centerPos[0]);
            //  Последняя точка в вычисленной дуге может иметь числовые ошибки.
            //  Точной конечной точкой является указанная (x, y). Заменить.
            m_state.curPos() = {x, y};
            if(arcPolygon.size())
                arcPolygon.last() = m_state.curPos();
            else
                arcPolygon.push_back(m_state.curPos());
        } break;
        case Single: // G74
            for(int c = 0; c < 4; ++c) {
                const double radius1 = sqrt(static_cast<double>(i) * static_cast<double>(i) + static_cast<double>(j) * static_cast<double>(j));
                const double radius2 = sqrt(pow(centerPos[c].X - x, 2.0) + pow(centerPos[c].Y - y, 2.0));
                // Убеждаемся, что радиус начала совпадает с радиусом конца.
                if(qAbs(radius2 - radius1) > (5e-4 * uScale)) // Недействительный центр.
                    continue;
                // Correct i and j and return true; as with multi-quadrant.
                i = centerPos[c].X - m_state.curPos().X;
                j = centerPos[c].Y - m_state.curPos().Y;
                // Углы
                const double start = atan2(-j, -i);
                const double stop = atan2(-centerPos[c].Y + y, -centerPos[c].X + x);
                const double angle = arcAngle(start, stop);
                if(angle < (pi + 1e-5) * 0.5) {
                    arcPolygon = arc(IntPoint(centerPos[c].X, centerPos[c].Y), radius1, start, stop);
                    // Replace with exact values
                    m_state.setCurPos({x, y});
                    if(arcPolygon.size())
                        arcPolygon.last() = m_state.curPos();
                    else
                        arcPolygon.push_back(m_state.curPos());
                    valid = true;
                }
            }
            if(!valid)
                qWarning() << QString("Invalid arc in line %1.").arg(m_lineNum) << gLine;
            break;
        default:
            m_state.setCurPos({x, y});
            m_path.push_back(m_state.curPos());
            return true;
            // break;
        }
        m_path.append(arcPolygon);
        // if (arcPolygon.size() > 0) {
        // for (Path::size_type i = 0, size = arcPolygon.size(); i < size && size; ++i) {
        // curPath.push_back(arcPolygon[i]); //polygon.emplace_back(arcPolygon); //push_back
        // }
        // }
        return true;
    }
    return false;
}

bool Parser::parseEndOfFile(const QString& gLine) {
    static const QRegExp match(QStringLiteral("^M[0]?[0123]\\*"));
    static const QRegExp match2(QStringLiteral("^D0?2M0?[02]\\*"));
    if(match.exactMatch(gLine) || match2.exactMatch(gLine)) {
        addPath();
        return true;
    }
    return false;
}

bool Parser::parseFormat(const QString& gLine) {
    const QStringList zeroOmissionModeList = QString("L|T").split("|");
    const QStringList coordinateValuesNotationList = QString("A|I").split("|");
    static const QRegExp match(QStringLiteral("^%FS([LT]?)([AI]?)X(\\d)(\\d)Y(\\d)(\\d)\\*%$"));
    if(match.exactMatch(gLine)) {
        switch(zeroOmissionModeList.indexOf(match.cap(1))) {
        case OmitLeadingZeros:
            m_state.format()->zeroOmisMode = OmitLeadingZeros;
            break;
#ifdef DEPRECATED
        case OmitTrailingZeros:
            m_state.format()->zeroOmisMode = OmitTrailingZeros;
            break;
#endif
        }
        switch(coordinateValuesNotationList.indexOf(match.cap(2))) {
        case AbsoluteNotation:
            m_state.format()->coordValueNotation = AbsoluteNotation;
            break;
#ifdef DEPRECATED
        case IncrementalNotation:
            m_state.format()->coordValueNotation = IncrementalNotation;
            break;
#endif
        }
        m_state.format()->xInteger = match.cap(3).toInt();
        m_state.format()->xDecimal = match.cap(4).toInt();
        m_state.format()->yInteger = match.cap(5).toInt();
        m_state.format()->yDecimal = match.cap(6).toInt();

        int intVal = m_state.format()->xInteger;
        if(intVal < 0 || intVal > 8)
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        intVal = m_state.format()->xDecimal;
        if(intVal < 0 || intVal > 8)
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        intVal = m_state.format()->yInteger;
        if(intVal < 0 || intVal > 8)
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        intVal = m_state.format()->yDecimal;
        if(intVal < 0 || intVal > 8)
            throw "Modifiers '" + gLine + "' XY is out of bounds 0в‰¤Nв‰¤7";
        return true;
    }
    return false;
}

bool Parser::parseGCode(const QString& gLine) {

    static const QRegExp match(QStringLiteral("^G([0]?[0-9]{2})\\*$"));
    if(match.exactMatch(gLine)) {
        switch(match.cap(1).toInt()) {
        case G01:
            m_state.setInterpolation(Linear);
            m_state.setGCode(G01);
            break;
        case G02:
            m_state.setInterpolation(ClockwiseCircular);
            m_state.setGCode(G02);
            break;
        case G03:
            m_state.setInterpolation(CounterclockwiseCircular);
            m_state.setGCode(G03);
            break;
        case G04:
            m_state.setGCode(G04);
            break;
        case G36:
            addPath();
            m_state.setRegion(On);
            m_state.setGCode(G36);
            m_state.setDCode(D02);
            break;
        case G37:
            addPath();
            m_state.setRegion(Off);
            m_state.setGCode(G37);
            break;
#ifdef DEPRECATED
        case G70:
            m_state.format()->unitMode = Inches;
            m_state.setGCode(G70);
            break;
        case G71:
            m_state.format()->unitMode = Millimeters;
            m_state.setGCode(G71);
            break;
#endif
        case G74:
            m_state.setQuadrant(Single);
            m_state.setGCode(G74);
            break;
        case G75:
            m_state.setQuadrant(Multi);
            m_state.setGCode(G75);
            break;
#ifdef DEPRECATED
        case G90:
            m_state.format()->coordValueNotation = AbsoluteNotation;
            m_state.setGCode(G90);
            break;
        case G91:
            m_state.format()->coordValueNotation = IncrementalNotation;
            m_state.setGCode(G91);
#endif
            break;
        default:
            qWarning() << "Erroror or deprecated G-code " << match.capturedTexts();
            break;
        }
        return true;
    }
    if(QRegExp("^G0?4(.*)$").exactMatch(gLine)) {
        m_state.setGCode(G04);
        return true;
    }
    return false;
}

bool Parser::parseImagePolarity(const QString& gLine) {
    static const QRegExp match(QStringLiteral("^%IP(POS|NEG)\\*%$"));
    static const QList<QString> slImagePolarity(QString("POS|NEG").split("|"));
    if(match.exactMatch(gLine)) {
        switch(slImagePolarity.indexOf(match.cap(1))) {
        case Positive:
            m_state.setImgPolarity(Positive);
            break;
        case Negative:
            m_state.setImgPolarity(Negative);
            break;
        }
        return true;
    }
    return false;
}

bool Parser::parseLineInterpolation(const QString& gLine) {
    // REGEX: r"^(?:G0?(1))?(?:X(-?\d+))?(?:Y(-?\d+))?(?:D0([123]))?\*$"
    static const QRegExp match(QStringLiteral("^(?:G0?(1))?(?=.*X([\\+-]?\\d+))?(?=.*Y([\\+-]?\\d+))?[XY]*[^DIJ]*(?:D0?([123]))?\\*$"));
    if(match.exactMatch(gLine)) {
        parsePosition(gLine);

        DCode dcode = m_state.dCode();
        if(!match.cap(2).isEmpty())
            dcode = static_cast<const DCode>(match.cap(2).toInt());

        switch(dcode) {
        case D01: // перемещение в указанную точку x-y с открытым затвором засветки
            m_state.setDCode(dcode);
            m_path.push_back(m_state.curPos());
            break;
        case D02: // перемещение в указанную точку x-y с закрытым затвором засветки
            addPath();
            m_state.setDCode(dcode);
            break;
        case D03: // перемещение в указанную точку x-y с закрытым затвором засветки и вспышка
            addPath();
            m_state.setDCode(dcode);
            addFlash();
            break;
        }
        return true;
    }
    return false;
}

bool Parser::parseDCode(const QString& gLine) {
    static const QRegExp match(QStringLiteral("^D0?([123])\\*$"));
    if(match.exactMatch(gLine)) {
        switch(match.cap(1).toInt()) {
        case D01:
            m_state.setDCode(D01);
            break;
        case D02:
            addPath();
            m_state.setDCode(D02);
            break;
        case D03:
            addPath();
            m_state.setDCode(D03);
            addFlash();
            break;
        }
        return true;
    }
    static const QRegExp match2(QStringLiteral("^(?:G54)?D(\\d+)\\*$"));
    if(match2.exactMatch(gLine)) {
        addPath();
        m_state.setAperture(match2.cap(1).toInt());
        m_state.setDCode(D02);
#ifdef DEPRECATED
        m_state.setGCode(G54);
#endif
        addPath();
        return true;
    }
    return false;
}

bool Parser::parseUnitMode(const QString& gLine) {
    static const QRegExp match(QStringLiteral("^%MO(IN|MM)\\*%$"));
    static const QList<QString> slUnitType(QString("IN|MM").split("|"));
    if(match.exactMatch(gLine)) {
        switch(slUnitType.indexOf(match.cap(1))) {
        case Inches:
            m_state.format()->unitMode = Inches;
            break;
        case Millimeters:
            m_state.format()->unitMode = Millimeters;
            break;
        }
        return true;
    }
    return false;
}

} // namespace Gerber

#include "moc_gbrparser.cpp"
