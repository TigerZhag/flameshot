// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "utils/ocrrunner.h"

#include "core/flameshotdaemon.h"
#include "utils/abstractlogger.h"
#include "utils/confighandler.h"

#include <QFileInfo>

OcrRunner::OcrRunner(const QPixmap& capture, QObject* parent)
  : QObject(parent)
  , m_capture(capture)
{}

void OcrRunner::start()
{
    if (!m_tempFile.open() || !m_capture.save(&m_tempFile, "PNG")) {
        notifyError(tr("Unable to save the selection for OCR."));
        return;
    }
    m_tempFile.close();

    QStringList args{ QFileInfo(m_tempFile).absoluteFilePath(),
                      QStringLiteral("stdout") };
    QString language = ConfigHandler().ocrLanguage();
    if (!language.isEmpty()) {
        args << QStringLiteral("-l") << language;
    }

    connect(&m_process,
            &QProcess::finished,
            this,
            &OcrRunner::onFinished);
    connect(&m_process,
            &QProcess::errorOccurred,
            this,
            &OcrRunner::onErrorOccurred);
    m_process.start(QStringLiteral("tesseract"), args);
}

void OcrRunner::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        QString error =
          QString::fromUtf8(m_process.readAllStandardError()).trimmed();
        notifyError(error.isEmpty() ? tr("OCR failed.")
                                    : tr("OCR failed: %1").arg(error));
        return;
    }

    QString text = QString::fromUtf8(m_process.readAllStandardOutput());
    text.remove(QLatin1Char('\r'));
    while (text.endsWith(QLatin1Char('\n'))) {
        text.chop(1);
    }
    if (text.isEmpty()) {
        AbstractLogger::warning() << tr("No text detected in the selection.");
    } else {
        FlameshotDaemon::copyToClipboard(text,
                                         tr("OCR text copied to clipboard."));
    }
    deleteLater();
}

void OcrRunner::onErrorOccurred(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        notifyError(
          tr("Unable to find the tesseract executable. Install tesseract "
             "to use OCR."));
    }
    // Other errors are reported through QProcess::finished.
}

void OcrRunner::notifyError(const QString& message)
{
    AbstractLogger::error() << message;
    deleteLater();
}
