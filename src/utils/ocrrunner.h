// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QPixmap>
#include <QProcess>
#include <QTemporaryFile>

/**
 * @brief Runs tesseract on a capture asynchronously and copies the recognized
 * text to the clipboard.
 *
 * It must run in a long-lived process (e.g. the daemon): it deletes itself
 * when the OCR process finishes or fails to start. When a parent is given,
 * ownership falls back to the parent if the process never finishes.
 */
class OcrRunner : public QObject
{
    Q_OBJECT
public:
    explicit OcrRunner(const QPixmap& capture, QObject* parent = nullptr);

    void start();

private slots:
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);

private:
    void notifyError(const QString& message);

    QPixmap m_capture;
    QTemporaryFile m_tempFile;
    QProcess m_process;
};
