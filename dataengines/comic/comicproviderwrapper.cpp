/*
 *   SPDX-FileCopyrightText: 2008 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "comicproviderwrapper.h"
#include "comicproviderkross.h"

#include <Kross/Core/Action>
#include <Kross/Core/Interpreter>
#include <Kross/Core/Manager>
#include <Plasma/Package>
#include <QDebug>
#include <QPainter>
#include <QStandardPaths>
#include <QTextCodec>
#include <QTimer>
#include <QUrl>

QStringList ComicProviderWrapper::mExtensions;

ImageWrapper::ImageWrapper(QObject *parent, const QByteArray &data)
    : QObject(parent)
    , mImage(QImage::fromData(data))
    , mRawData(data)
{
    resetImageReader();
}

QImage ImageWrapper::image() const
{
    return mImage;
}

void ImageWrapper::setImage(const QImage &image)
{
    mImage = image;
    mRawData.clear();

    resetImageReader();
}

QByteArray ImageWrapper::rawData() const
{
    if (mRawData.isNull()) {
        QBuffer buffer(&mRawData);
        mImage.save(&buffer);
    }

    return mRawData;
}

void ImageWrapper::setRawData(const QByteArray &rawData)
{
    mRawData = rawData;
    mImage = QImage::fromData(mRawData);

    resetImageReader();
}

void ImageWrapper::resetImageReader()
{
    if (mBuffer.isOpen()) {
        mBuffer.close();
    }
    rawData(); // to update the rawData if needed
    mBuffer.setBuffer(&mRawData);
    mBuffer.open(QIODevice::ReadOnly);
    mImageReader.setDevice(&mBuffer);
}

int ImageWrapper::imageCount() const
{
    return mImageReader.imageCount();
}

QImage ImageWrapper::read()
{
    return mImageReader.read();
}

DateWrapper::DateWrapper(QObject *parent, const QDate &date)
    : QObject(parent)
    , mDate(date)
{
}

QDate DateWrapper::date() const
{
    return mDate;
}

void DateWrapper::setDate(const QDate &date)
{
    mDate = date;
}

QDate DateWrapper::fromVariant(const QVariant &variant)
{
    if (variant.type() == QVariant::Date || variant.type() == QVariant::DateTime) {
        return variant.toDate();
    } else if (variant.type() == QVariant::String) {
        return QDate::fromString(variant.toString(), Qt::ISODate);
    } else {
        DateWrapper *dw = qobject_cast<DateWrapper *>(variant.value<QObject *>());
        if (dw) {
            return dw->date();
        }
    }
    return QDate();
}

QObject *DateWrapper::addDays(int ndays)
{
    return new DateWrapper(this, mDate.addDays(ndays));
}

QObject *DateWrapper::addMonths(int nmonths)
{
    return new DateWrapper(this, mDate.addMonths(nmonths));
}

QObject *DateWrapper::addYears(int nyears)
{
    return new DateWrapper(this, mDate.addYears(nyears));
}

int DateWrapper::day() const
{
    return mDate.day();
}

int DateWrapper::dayOfWeek() const
{
    return mDate.dayOfWeek();
}

int DateWrapper::dayOfYear() const
{
    return mDate.dayOfYear();
}

int DateWrapper::daysInMonth() const
{
    return mDate.daysInMonth();
}

int DateWrapper::daysInYear() const
{
    return mDate.daysInYear();
}

int DateWrapper::daysTo(const QVariant d) const
{
    return mDate.daysTo(fromVariant(d));
}

bool DateWrapper::isNull() const
{
    return mDate.isNull();
}

bool DateWrapper::isValid() const
{
    return mDate.isValid();
}

int DateWrapper::month() const
{
    return mDate.month();
}

bool DateWrapper::setDate(int year, int month, int day)
{
    return mDate.setDate(year, month, day);
}

int DateWrapper::toJulianDay() const
{
    return mDate.toJulianDay();
}

QString DateWrapper::toString(const QString &format) const
{
    return mDate.toString(format);
}

QString DateWrapper::toString(int format) const
{
    return mDate.toString((Qt::DateFormat)format);
}

int DateWrapper::weekNumber() const
{
    return mDate.weekNumber();
}

int DateWrapper::year() const
{
    return mDate.year();
}

StaticDateWrapper::StaticDateWrapper(QObject *parent)
    : QObject(parent)
{
}

QObject *StaticDateWrapper::currentDate()
{
    return new DateWrapper(this, QDate::currentDate());
}

QObject *StaticDateWrapper::fromJulianDay(int jd)
{
    return new DateWrapper(this, QDate::fromJulianDay(jd));
}

QObject *StaticDateWrapper::fromString(const QString &string, int format)
{
    return new DateWrapper(this, QDate::fromString(string, (Qt::DateFormat)format));
}

QObject *StaticDateWrapper::fromString(const QString &string, const QString &format)
{
    return new DateWrapper(this, QDate::fromString(string, format));
}

bool StaticDateWrapper::isLeapYear(int year)
{
    return QDate::isLeapYear(year);
}

bool StaticDateWrapper::isValid(int year, int month, int day)
{
    return QDate::isValid(year, month, day);
}

QString StaticDateWrapper::longDayName(int weekday)
{
    return QLocale::system().dayName(weekday, QLocale::LongFormat);
}

QString StaticDateWrapper::longMonthName(int month)
{
    return QLocale::system().monthName(month, QLocale::LongFormat);
}

QString StaticDateWrapper::shortDayName(int weekday)
{
    return QLocale::system().dayName(weekday, QLocale::ShortFormat);
}

QString StaticDateWrapper::shortMonthName(int month)
{
    return QLocale::system().monthName(month, QLocale::ShortFormat);
}

ComicProviderWrapper::ComicProviderWrapper(ComicProviderKross *parent)
    : QObject(parent)
    , mAction(nullptr)
    , mProvider(parent)
    , mKrossImage(nullptr)
    , mPackage(nullptr)
    , mRequests(0)
    , mIdentifierSpecified(false)
    , mIsLeftToRight(true)
    , mIsTopToBottom(true)
{
    QTimer::singleShot(0, this, &ComicProviderWrapper::init);
}

ComicProviderWrapper::~ComicProviderWrapper()
{
    delete mPackage;
}

void ComicProviderWrapper::init()
{
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                QLatin1String("plasma/comics/") + mProvider->pluginName() + QLatin1Char('/'),
                                                QStandardPaths::LocateDirectory);
    // qDebug() << "ComicProviderWrapper::init() package is" << mProvider->pluginName() << " at " <<  path;

    if (!path.isEmpty()) {
        mPackage = new KPackage::Package(ComicProviderKross::packageStructure());
        mPackage->setPath(path);

        if (mPackage->isValid()) {
            // QString mainscript = mPackage->filePath("mainscript")
            // doesn't work because the mainscript file can be a different Kross script type ending with main.es or main.kjs etc.
            // Maybe this should be added to Plasma::Package or maybe directly change the packagestructure mainscript definition as "main.es" and avoid all
            // this. https://techbase.kde.org/Development/Tutorials/Plasma4/ComicPlugin#Package_Structure has main.es defined as mainscript. Also
            // Package::isValid() fails because the mainscript search fails to find the "main" file from mainscript.

            QString mainscript = mPackage->filePath("scripts") + QLatin1String("/main");
            QFileInfo info(mainscript);
            for (int i = 0; i < extensions().count() && !info.exists(); ++i) {
                info.setFile(mainscript + extensions().value(i));
                // qDebug() << "ComicProviderWrapper::init() mainscript found as" << info.filePath();
            }

            if (info.exists()) {
                mAction = new Kross::Action(parent(), mProvider->pluginName());
                if (mAction) {
                    mAction->addObject(this, QLatin1String("comic"));
                    mAction->addObject(new StaticDateWrapper(this), QLatin1String("date"));
                    mAction->setFile(info.filePath());
                    mAction->trigger();
                    mFunctions = mAction->functionNames();

                    mIdentifierSpecified = !mProvider->isCurrent();
                    setIdentifierToDefault();
                    callFunction(QLatin1String("init"));
                }
            }
        }
    }
}

const QStringList &ComicProviderWrapper::extensions() const
{
    if (mExtensions.isEmpty()) {
        Kross::InterpreterInfo *info;
        QStringList list;
        QString wildcards;

        foreach (const QString &interpretername, Kross::Manager::self().interpreters()) {
            info = Kross::Manager::self().interpreterInfo(interpretername);
            wildcards = info->wildcard();
            wildcards.remove(QLatin1Char('*'));
            mExtensions << wildcards.split(QLatin1Char(' '));
        }
    }
    return mExtensions;
}

ComicProvider::IdentifierType ComicProviderWrapper::identifierType() const
{
    ComicProvider::IdentifierType result = ComicProvider::StringIdentifier;
    const QString type = mProvider->description().value(QLatin1String("X-KDE-PlasmaComicProvider-SuffixType"));
    if (type == QLatin1String("Date")) {
        result = ComicProvider::DateIdentifier;
    } else if (type == QLatin1String("Number")) {
        result = ComicProvider::NumberIdentifier;
    } else if (type == QLatin1String("String")) {
        result = ComicProvider::StringIdentifier;
    }
    return result;
}

QImage ComicProviderWrapper::comicImage()
{
    ImageWrapper *img = qobject_cast<ImageWrapper *>(callFunction(QLatin1String("image")).value<QObject *>());
    if (functionCalled() && img) {
        return img->image();
    }
    if (mKrossImage) {
        return mKrossImage->image();
    }
    return QImage();
}

QVariant ComicProviderWrapper::identifierToScript(const QVariant &identifier)
{
    if (identifierType() == ComicProvider::DateIdentifier && identifier.type() != QVariant::Bool) {
        return QVariant::fromValue(qobject_cast<QObject *>(new DateWrapper(this, identifier.toDate())));
    }
    return identifier;
}

QVariant ComicProviderWrapper::identifierFromScript(const QVariant &identifier) const
{
    QVariant result = identifier;
    if (identifier.type() != QVariant::Bool) {
        switch (identifierType()) {
        case DateIdentifier:
            result = DateWrapper::fromVariant(identifier);
            break;
        case NumberIdentifier:
            result = identifier.toInt();
            break;
        case StringIdentifier:
            result = identifier.toString();
            break;
        }
    }
    return result;
}

void ComicProviderWrapper::checkIdentifier(QVariant *identifier)
{
    switch (identifierType()) {
    case DateIdentifier:
        if (!mLastIdentifier.isNull() && !identifier->isNull() && (!mIdentifierSpecified || identifier->toDate() > mLastIdentifier.toDate())) {
            *identifier = mLastIdentifier;
        }
        if (!mFirstIdentifier.isNull() && !identifier->isNull() && identifier->toDate() < mFirstIdentifier.toDate()) {
            *identifier = mFirstIdentifier;
        }
        break;
    case NumberIdentifier:
        if (!mLastIdentifier.isNull() && !identifier->isNull() && (!mIdentifierSpecified || identifier->toInt() > mLastIdentifier.toInt())) {
            *identifier = mLastIdentifier;
        }
        if (!mFirstIdentifier.isNull() && !identifier->isNull() && identifier->toInt() < mFirstIdentifier.toInt()) {
            *identifier = mFirstIdentifier;
        }
        break;
    case StringIdentifier:
        if (!mLastIdentifier.isNull() && !mLastIdentifier.toString().isEmpty() && !mIdentifierSpecified) {
            *identifier = mLastIdentifier;
        }
        break;
    }
}

void ComicProviderWrapper::setIdentifierToDefault()
{
    switch (identifierType()) {
    case DateIdentifier:
        mIdentifier = mProvider->requestedDate();
        mLastIdentifier = QDate::currentDate();
        break;
    case NumberIdentifier:
        mIdentifier = mProvider->requestedNumber();
        mFirstIdentifier = 1;
        break;
    case StringIdentifier:
        mIdentifier = mProvider->requestedString();
        break;
    }
}

bool ComicProviderWrapper::identifierSpecified() const
{
    return mIdentifierSpecified;
}

bool ComicProviderWrapper::isLeftToRight() const
{
    return mIsLeftToRight;
}

void ComicProviderWrapper::setLeftToRight(bool ltr)
{
    mIsLeftToRight = ltr;
}

bool ComicProviderWrapper::isTopToBottom() const
{
    return mIsTopToBottom;
}

void ComicProviderWrapper::setTopToBottom(bool ttb)
{
    mIsTopToBottom = ttb;
}

QString ComicProviderWrapper::textCodec() const
{
    return QString::fromLatin1(mTextCodec);
}

void ComicProviderWrapper::setTextCodec(const QString &textCodec)
{
    mTextCodec = textCodec.toLatin1();
}

QString ComicProviderWrapper::comicAuthor() const
{
    return mProvider->comicAuthor();
}

void ComicProviderWrapper::setComicAuthor(const QString &author)
{
    mProvider->setComicAuthor(author);
}

QString ComicProviderWrapper::websiteUrl() const
{
    return mWebsiteUrl;
}

void ComicProviderWrapper::setWebsiteUrl(const QString &websiteUrl)
{
    mWebsiteUrl = websiteUrl;
}

QString ComicProviderWrapper::shopUrl() const
{
    return mShopUrl;
}

void ComicProviderWrapper::setShopUrl(const QString &shopUrl)
{
    mShopUrl = shopUrl;
}

QString ComicProviderWrapper::title() const
{
    return mTitle;
}

void ComicProviderWrapper::setTitle(const QString &title)
{
    mTitle = title;
}

QString ComicProviderWrapper::additionalText() const
{
    return mAdditionalText;
}

void ComicProviderWrapper::setAdditionalText(const QString &additionalText)
{
    mAdditionalText = additionalText;
}

QVariant ComicProviderWrapper::identifier()
{
    return identifierToScript(mIdentifier);
}

void ComicProviderWrapper::setIdentifier(const QVariant &identifier)
{
    mIdentifier = identifierFromScript(identifier);
    checkIdentifier(&mIdentifier);
}

QVariant ComicProviderWrapper::nextIdentifier()
{
    return identifierToScript(mNextIdentifier);
}

void ComicProviderWrapper::setNextIdentifier(const QVariant &nextIdentifier)
{
    mNextIdentifier = identifierFromScript(nextIdentifier);
    if (mNextIdentifier == mIdentifier) {
        mNextIdentifier.clear();
        qWarning() << "Next identifier is the same as the current one, clearing next identifier.";
    }
}

QVariant ComicProviderWrapper::previousIdentifier()
{
    return identifierToScript(mPreviousIdentifier);
}

void ComicProviderWrapper::setPreviousIdentifier(const QVariant &previousIdentifier)
{
    mPreviousIdentifier = identifierFromScript(previousIdentifier);
    if (mPreviousIdentifier == mIdentifier) {
        mPreviousIdentifier.clear();
        qWarning() << "Previous identifier is the same as the current one, clearing previous identifier.";
    }
}

QVariant ComicProviderWrapper::firstIdentifier()
{
    return identifierToScript(mFirstIdentifier);
}

void ComicProviderWrapper::setFirstIdentifier(const QVariant &firstIdentifier)
{
    switch (identifierType()) {
    case DateIdentifier:
        mProvider->setFirstStripDate(DateWrapper::fromVariant(firstIdentifier));
        break;
    case NumberIdentifier:
        mProvider->setFirstStripNumber(firstIdentifier.toInt());
        break;
    case StringIdentifier:
        break;
    }
    mFirstIdentifier = identifierFromScript(firstIdentifier);
    checkIdentifier(&mIdentifier);
}

QVariant ComicProviderWrapper::lastIdentifier()
{
    return identifierToScript(mLastIdentifier);
}

void ComicProviderWrapper::setLastIdentifier(const QVariant &lastIdentifier)
{
    mLastIdentifier = identifierFromScript(lastIdentifier);
    checkIdentifier(&mIdentifier);
}

QVariant ComicProviderWrapper::identifierVariant() const
{
    return mIdentifier;
}

QVariant ComicProviderWrapper::firstIdentifierVariant() const
{
    return mFirstIdentifier;
}

QVariant ComicProviderWrapper::lastIdentifierVariant() const
{
    return mLastIdentifier;
}

QVariant ComicProviderWrapper::nextIdentifierVariant() const
{
    // either handle both previousIdentifier and nextIdentifier or handle none
    if (mPreviousIdentifier.isNull() && mNextIdentifier.isNull()) {
        switch (identifierType()) {
        case DateIdentifier:
            if ((mLastIdentifier.isNull() && mIdentifier.toDate() < QDate::currentDate())
                || (!mLastIdentifier.isNull() && mIdentifier.toDate() < mLastIdentifier.toDate())) {
                return mIdentifier.toDate().addDays(1);
            } else {
                return false;
            }
        case NumberIdentifier:
            if (mLastIdentifier.isNull() || mIdentifier.toInt() < mLastIdentifier.toInt()) {
                return mIdentifier.toInt() + 1;
            } else {
                return false;
            }
        case StringIdentifier:
            break;
        }
        // check if the nextIdentifier is correct
    } else if (!mNextIdentifier.isNull()) {
        // no nextIdentifier if mIdentifier == mLastIdentifier or if no identifier has been specified
        switch (identifierType()) {
        case DateIdentifier:
            if ((!mLastIdentifier.isNull() && (mIdentifier.toDate() == mLastIdentifier.toDate())) || !mIdentifierSpecified) {
                return false;
            }
            break;
        case NumberIdentifier:
            if ((!mLastIdentifier.isNull() && (mIdentifier.toInt() == mLastIdentifier.toInt())) || !mIdentifierSpecified) {
                return false;
            }
            break;
        case StringIdentifier:
            if (!mIdentifierSpecified) {
                return false;
            }
            break;
        }
    }
    return mNextIdentifier;
}

QVariant ComicProviderWrapper::previousIdentifierVariant() const
{
    // either handle both previousIdentifier and nextIdentifier or handle none
    if (mPreviousIdentifier.isNull() && mNextIdentifier.isNull()) {
        switch (identifierType()) {
        case DateIdentifier:
            if (mFirstIdentifier.isNull() || mIdentifier.toDate() > mFirstIdentifier.toDate()) {
                return mIdentifier.toDate().addDays(-1);
            } else {
                return false;
            }
        case NumberIdentifier:
            if ((mFirstIdentifier.isNull() && mIdentifier.toInt() > 1) || (!mFirstIdentifier.isNull() && mIdentifier.toInt() > mFirstIdentifier.toInt())) {
                return mIdentifier.toInt() - 1;
            } else {
                return false;
            }
        case StringIdentifier:
            break;
        }
    } else if (!mPreviousIdentifier.isNull()) {
        // no previousIdentifier if mIdentifier == mFirstIdentifier
        switch (identifierType()) {
        case DateIdentifier:
            if (!mFirstIdentifier.isNull() && (mIdentifier.toDate() == mFirstIdentifier.toDate())) {
                return false;
            }
            break;
        case NumberIdentifier:
            if (!mFirstIdentifier.isNull() && (mIdentifier.toInt() == mFirstIdentifier.toInt())) {
                return false;
            }
            break;
        case StringIdentifier:
            break;
        }
    }
    return mPreviousIdentifier;
}

void ComicProviderWrapper::pageRetrieved(int id, const QByteArray &data)
{
    --mRequests;
    if (id == Image) {
        mKrossImage = new ImageWrapper(this, data);
        callFunction(QLatin1String("pageRetrieved"), QVariantList() << id << QVariant::fromValue(qobject_cast<QObject *>(mKrossImage)));
        if (mRequests < 1) { // Don't finish if we still have pageRequests
            finished();
        }
    } else {
        QTextCodec *codec = nullptr;
        if (!mTextCodec.isEmpty()) {
            codec = QTextCodec::codecForName(mTextCodec);
        }
        if (!codec) {
            codec = QTextCodec::codecForHtml(data);
        }
        QString html = codec->toUnicode(data);

        callFunction(QLatin1String("pageRetrieved"), QVariantList() << id << html);
    }
}

void ComicProviderWrapper::pageError(int id, const QString &message)
{
    --mRequests;
    callFunction(QLatin1String("pageError"), QVariantList() << id << message);
    if (!functionCalled()) {
        Q_EMIT mProvider->error(mProvider);
    }
}

void ComicProviderWrapper::redirected(int id, const QUrl &newUrl)
{
    --mRequests;
    callFunction(QLatin1String("redirected"), QVariantList() << id << newUrl);
    if (mRequests < 1) { // Don't finish while there are still requests
        finished();
    }
}

void ComicProviderWrapper::finished() const
{
    qDebug() << QString::fromLatin1("Author").leftJustified(22, QLatin1Char('.')) << comicAuthor();
    qDebug() << QString::fromLatin1("Website URL").leftJustified(22, QLatin1Char('.')) << mWebsiteUrl;
    qDebug() << QString::fromLatin1("Shop URL").leftJustified(22, QLatin1Char('.')) << mShopUrl;
    qDebug() << QString::fromLatin1("Title").leftJustified(22, QLatin1Char('.')) << mTitle;
    qDebug() << QString::fromLatin1("Additional Text").leftJustified(22, QLatin1Char('.')) << mAdditionalText;
    qDebug() << QString::fromLatin1("Identifier").leftJustified(22, QLatin1Char('.')) << mIdentifier;
    qDebug() << QString::fromLatin1("First Identifier").leftJustified(22, QLatin1Char('.')) << mFirstIdentifier;
    qDebug() << QString::fromLatin1("Last Identifier").leftJustified(22, QLatin1Char('.')) << mLastIdentifier;
    qDebug() << QString::fromLatin1("Next Identifier").leftJustified(22, QLatin1Char('.')) << mNextIdentifier;
    qDebug() << QString::fromLatin1("Previous Identifier").leftJustified(22, QLatin1Char('.')) << mPreviousIdentifier;
    Q_EMIT mProvider->finished(mProvider);
}

void ComicProviderWrapper::error() const
{
    Q_EMIT mProvider->error(mProvider);
}

void ComicProviderWrapper::requestPage(const QString &url, int id, const QVariantMap &infos)
{
    QMap<QString, QString> map;

    foreach (const QString &key, infos.keys()) {
        map[key] = infos[key].toString();
    }
    mProvider->requestPage(QUrl(url), id, map);
    ++mRequests;
}

void ComicProviderWrapper::requestRedirectedUrl(const QString &url, int id, const QVariantMap &infos)
{
    QMap<QString, QString> map;

    foreach (const QString &key, infos.keys()) {
        map[key] = infos[key].toString();
    }
    mProvider->requestRedirectedUrl(QUrl(url), id, map);
    ++mRequests;
}

bool ComicProviderWrapper::functionCalled() const
{
    return mFuncFound;
}

QVariant ComicProviderWrapper::callFunction(const QString &name, const QVariantList &args)
{
    if (mAction) {
        mFuncFound = mFunctions.contains(name);
        if (mFuncFound) {
            return mAction->callFunction(name, args);
        }
    }
    return QVariant();
}

void ComicProviderWrapper::combine(const QVariant &image, PositionType position)
{
    if (!mKrossImage) {
        return;
    }

    QImage header;
    if (image.type() == QVariant::String) {
        const QString path(mPackage->filePath("images", image.toString()));
        if (QFile::exists(path)) {
            header = QImage(path);
        } else {
            return;
        }
    } else {
        ImageWrapper *img = qobject_cast<ImageWrapper *>(image.value<QObject *>());
        if (img) {
            header = img->image();
        } else {
            return;
        }
    }
    const QImage comic = mKrossImage->image();
    int height = 0;
    int width = 0;

    switch (position) {
    case Top:
    case Bottom:
        height = header.height() + comic.height();
        width = (header.width() >= comic.width()) ? header.width() : comic.width();
        break;
    case Left:
    case Right:
        height = (header.height() >= comic.height()) ? header.height() : comic.height();
        width = header.width() + comic.width();
        break;
    }

    QImage img = QImage(QSize(width, height), QImage::Format_RGB32);
    img.fill(header.pixel(QPoint(0, 0)));

    QPainter painter(&img);

    // center and draw the Images
    QPoint headerPos;
    QPoint comicPos;

    switch (position) {
    case Top:
        headerPos = QPoint(((width - header.width()) / 2), 0);
        comicPos = QPoint(((width - comic.width()) / 2), header.height());
        break;
    case Bottom:
        headerPos = QPoint(((width - header.width()) / 2), comic.height());
        comicPos = QPoint(((width - comic.width()) / 2), 0);
        break;
    case Left:
        headerPos = QPoint(0, ((height - header.height()) / 2));
        comicPos = QPoint(header.width(), ((height - comic.height()) / 2));
        break;
    case Right:
        headerPos = QPoint(comic.width(), ((height - header.height()) / 2));
        comicPos = QPoint(0, ((height - comic.height()) / 2));
        break;
    }
    painter.drawImage(headerPos, header);
    painter.drawImage(comicPos, comic);
    mKrossImage->setImage(img);
}

QObject *ComicProviderWrapper::image()
{
    return qobject_cast<QObject *>(mKrossImage);
}
