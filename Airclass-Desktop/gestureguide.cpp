#include "gestureguide.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QPixmap>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

            // File paths - using proper path resolution
            const QString GESTURE_MAP_FILENAME = "gesturemap.txt";
const QString GESTURE_MAP_SUBFOLDER = "gestureMap";
const QString GESTURE_IMAGE_PREFIX = ":/gestureGuidePictures/gestureGuidePictures/";

// Constructor no longer takes GestureProcessor
GestureGuide::GestureGuide(QWidget *parent)
    : QDialog(parent) // m_gestureProcessor removed from initializer list
{
    setWindowTitle(tr("Gesture Guide"));
    setMinimumSize(600, 700);

    initializeCustomizableActions(); // Initialize the list of actions for UI
    loadGestureMap(); // Load from file
    setupUI();
}

GestureGuide::~GestureGuide()
{
}

void GestureGuide::initializeCustomizableActions()
{
    m_customizableActionsList.clear();
    m_customizableActionsList << "next_slide"
                              << "prev_slide"
                              << "zoom_in"
                              << "zoom_out"
                              << "break"
                              << "attendance";
    // "None" will be added explicitly to the QComboBox during UI setup.
    qDebug() << "GestureGuide: Initialized customizable actions:" << m_customizableActionsList;
}

QString GestureGuide::getGestureMapPath()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString path = QDir(appDir).filePath(GESTURE_MAP_SUBFOLDER + "/" + GESTURE_MAP_FILENAME);
    return path;
}

void GestureGuide::loadGestureMap()
{
    m_allGestures.clear();
    QString gestureMapPath = getGestureMapPath();
    QFile file(gestureMapPath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open gesture map file:" << gestureMapPath << file.errorString();
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not load gesture map from:\n%1\n\n"
                                "Please ensure the file exists or the application has proper permissions.\n"
                                "Searched locations:\n"
                                "- Application directory: %2\n"
                                "- Current directory: %3")
                                 .arg(gestureMapPath)
                                 .arg(QCoreApplication::applicationDirPath())
                                 .arg(QDir::currentPath()));
        return;
    }

    QTextStream in(&file);
    bool inCustomSection = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) {
            if (!inCustomSection) {
                inCustomSection = true;
                qDebug() << "Switching to customizable gestures section";
            }
            continue;
        }
        if (line.startsWith("#")) continue;

        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.isEmpty()) continue;

        GestureInfo gesture;
        gesture.id = parts[0];
        gesture.name = gesture.id;
        gesture.name = gesture.name.replace("_", " ");
        QStringList words = gesture.name.split(' ', Qt::SkipEmptyParts);
        QStringList capitalizedWords;
        for (const QString& word : words) {
            if (!word.isEmpty()) {
                QString capitalizedWord = word.toLower();
                capitalizedWord[0] = capitalizedWord[0].toUpper();
                capitalizedWords.append(capitalizedWord);
            }
        }
        gesture.name = capitalizedWords.join(" ");
        gesture.imagePath = GESTURE_IMAGE_PREFIX + gesture.id + ".png";

        if (!inCustomSection) { // Predefined gestures section
            gesture.isRequired = true;
            if (parts.size() > 1) {
                gesture.action = parts[1];
                gesture.description = tr("Action: %1").arg(gesture.action);
                if (parts.size() > 2) {
                    bool ok;
                    int count = parts[2].toInt(&ok);
                    if (ok) gesture.repeatCount = count;
                    else {
                        qWarning() << "Could not parse repeat count for" << gesture.id << ":" << parts[2];
                        gesture.repeatCount = 1;
                    }
                    gesture.description += tr(", Repeats: %1").arg(gesture.repeatCount);
                } else gesture.repeatCount = 1;
            } else {
                gesture.action = "";
                gesture.repeatCount = 1;
                gesture.description = tr("No action defined");
            }
        } else { // Customizable gestures section
            gesture.isRequired = false;
            if (parts.size() > 1) {
                gesture.action = parts[1];
                if (parts.size() > 2) {
                    bool ok;
                    int count = parts[2].toInt(&ok);
                    if (ok) gesture.repeatCount = count;
                    else {
                        qWarning() << "Could not parse repeat count for customizable gesture" << gesture.id << ":" << parts[2];
                        gesture.repeatCount = 1;
                    }
                } else gesture.repeatCount = 1;
            } else {
                gesture.action = "";
                gesture.repeatCount = 1;
            }
            // NO LONGER OVERRIDING FROM GestureProcessor
        }
        m_allGestures.append(gesture);
    }
    file.close();
    qDebug() << "Loaded" << m_allGestures.size() << "gestures total from:" << gestureMapPath;
    int predefinedCount = 0, customCount = 0;
    for (const auto& g : m_allGestures) { (g.isRequired ? predefinedCount : customCount)++; }
    qDebug() << "Predefined gestures:" << predefinedCount << ", Customizable gestures:" << customCount;
}

void GestureGuide::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->addTab(createGesturesTab(true), tr("Pre-configured Gestures"));
    m_tabWidget->addTab(createGesturesTab(false), tr("Customizable Gestures"));
    mainLayout->addWidget(m_tabWidget);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton *saveButton = new QPushButton(tr("Save Configuration"), this);
    saveButton->setStyleSheet(R"(
        QPushButton {
            padding: 8px 16px;
            background-color: #2ecc71; color: white; border: none;
            border-radius: 6px; font-size: 14px; font-weight: bold;
        }
        QPushButton:hover { background-color: #27ae60; }
        QPushButton:pressed { background-color: #27ae60; }
    )");
    connect(saveButton, &QPushButton::clicked, this, &GestureGuide::saveGestureConfiguration);

    QPushButton *closeButton = new QPushButton(tr("Close"), this);
    closeButton->setStyleSheet(R"(
        QPushButton {
            padding: 8px 16px;
            background-color: #6c757d; color: white; border: none;
            border-radius: 6px; font-size: 14px; font-weight: bold;
        }
        QPushButton:hover { background-color: #5a6268; }
        QPushButton:pressed { background-color: #545b62; }
    )");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addWidget(saveButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

QWidget* GestureGuide::createGesturesTab(bool forRequiredGestures)
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    QLabel *title = new QLabel(forRequiredGestures ? tr("Pre-configured Gestures") : tr("Customizable Gestures"), tab);
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    layout->addWidget(title);

    QScrollArea *scrollArea = new QScrollArea(tab);
    QWidget *contentWidget = new QWidget(scrollArea);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(10);

    for (const auto &gesture : m_allGestures) {
        if (gesture.isRequired == forRequiredGestures) {
            QWidget *gestureWidget = new QWidget(contentWidget);
            QHBoxLayout *gestureLayout = new QHBoxLayout(gestureWidget);
            populateGestureItem(gestureLayout, gesture, tab);
            contentLayout->addWidget(gestureWidget);
            if(!forRequiredGestures) {
                QFrame* separator = new QFrame();
                separator->setFrameShape(QFrame::HLine);
                separator->setFrameShadow(QFrame::Sunken);
                contentLayout->addWidget(separator);
            }
        }
    }
    contentLayout->addStretch();
    contentWidget->setLayout(contentLayout);
    scrollArea->setWidget(contentWidget);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);
    tab->setLayout(layout);
    return tab;
}

void GestureGuide::populateGestureItem(QHBoxLayout* itemLayout, const GestureInfo &gesture, QWidget* parentTab) {
    itemLayout->setSpacing(15);
    QLabel *imageLabel = new QLabel(parentTab);
    QPixmap pixmap(gesture.imagePath);
    if (pixmap.isNull()) {
        qWarning() << "Could not load image:" << gesture.imagePath;
        imageLabel->setText(tr("No Image"));
        imageLabel->setFixedSize(64, 64);
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setStyleSheet("border: 1px solid #ccc; color: #888;");
    } else {
        imageLabel->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    imageLabel->setToolTip(gesture.id);
    itemLayout->addWidget(imageLabel);

    QLabel *nameLabel = new QLabel(gesture.name, parentTab);
    nameLabel->setStyleSheet("font-weight: bold; font-size: 14px; min-width: 100px;");
    itemLayout->addWidget(nameLabel);

    if (gesture.isRequired) {
        QLabel *descLabel = new QLabel(gesture.description, parentTab);
        descLabel->setWordWrap(true);
        itemLayout->addWidget(descLabel, 1);
    } else { // Customizable gestures
        // REMOVED: if (!m_gestureProcessor) { ... return; }

        QComboBox *actionCombo = new QComboBox(parentTab);
        actionCombo->addItem(tr("None"), ""); // UserData is an empty string for "None"
        for (const QString& action : m_customizableActionsList) {
            QString displayName = action;
            displayName = displayName.replace("_", " ");
            if (!displayName.isEmpty()) {
                displayName[0] = displayName[0].toUpper();
            }
            actionCombo->addItem(displayName, action); // Display name, actual action string
        }

        if (gesture.action.isEmpty()) {
            actionCombo->setCurrentIndex(0); // "None"
        } else {
            int index = actionCombo->findData(gesture.action);
            if (index != -1) {
                actionCombo->setCurrentIndex(index);
            } else { // Action from file not in standard list, add dynamically
                qDebug() << "Custom action" << gesture.action << "from file not in standard list. Adding dynamically.";
                QString displayName = gesture.action;
                displayName = displayName.replace("_", " ");
                if (!displayName.isEmpty()) {
                    displayName[0] = displayName[0].toUpper();
                }
                actionCombo->addItem(displayName, gesture.action);
                actionCombo->setCurrentIndex(actionCombo->count() - 1);
            }
        }
        actionCombo->setMinimumWidth(150);
        m_customActionCombos[gesture.id] = actionCombo;

        QSpinBox *repeatSpin = new QSpinBox(parentTab);
        repeatSpin->setRange(1, 10);
        repeatSpin->setValue(gesture.repeatCount > 0 ? gesture.repeatCount : 1);
        repeatSpin->setPrefix(tr("Repeat: "));
        m_customRepeatSpins[gesture.id] = repeatSpin;

        connect(actionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this, gestureId = gesture.id, repeatSpin, actionCombo](int index) {
                    QString newAction = actionCombo->itemData(index).toString();
                    int currentRepeat = repeatSpin->value();
                    updateGestureConfig(gestureId, newAction, currentRepeat);
                    // REMOVED: m_gestureProcessor interaction
                });

        connect(repeatSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                [this, gestureId = gesture.id, actionCombo](int newRepeat) {
                    QString currentAction = actionCombo->itemData(actionCombo->currentIndex()).toString();
                    updateGestureConfig(gestureId, currentAction, newRepeat);
                    // REMOVED: m_gestureProcessor interaction
                });

        itemLayout->addWidget(actionCombo);
        itemLayout->addWidget(repeatSpin);
        itemLayout->addStretch();
    }
}

void GestureGuide::updateGestureConfig(const QString& gestureId, const QString& newAction, int newRepeatCount) {
    for (GestureInfo& gesture : m_allGestures) {
        if (gesture.id == gestureId) {
            gesture.action = newAction;
            gesture.repeatCount = newRepeatCount > 0 ? newRepeatCount : 1; // Ensure repeat is at least 1
            qDebug() << "GestureGuide: Updated in-memory config for" << gestureId << "to action:" << newAction << "repeats:" << gesture.repeatCount;
            return;
        }
    }
}

void GestureGuide::saveGestureConfiguration()
{
    qDebug() << "GestureGuide: Attempting to save gesture configuration...";
    QString saveFilePath = getGestureMapPath();
    QStringList linesToSave;

    for (const GestureInfo &gesture : m_allGestures) {
        if (gesture.isRequired) {
            QString line = gesture.id;
            if (!gesture.action.isEmpty()) {
                line += " " + gesture.action;
                if (gesture.repeatCount > 1) {
                    line += " " + QString::number(gesture.repeatCount);
                }
            }
            linesToSave.append(line);
        }
    }
    linesToSave.append("");

    for (const GestureInfo &gesture : m_allGestures) {
        if (!gesture.isRequired) {
            QString line = gesture.id;
            if (!gesture.action.isEmpty()) { // Only save action if it's not "None"
                line += " " + gesture.action;
                line += " " + QString::number(gesture.repeatCount > 0 ? gesture.repeatCount : 1);
            }
            // If action is "None" (empty), only the gesture.id is saved for customizable gestures.
            // This allows it to appear in the customizable list next time without a default action.
            // If you want to completely omit lines for "None" actions, add an `else` here or filter.
            linesToSave.append(line);
        }
    }

    QFileInfo fileInfo(saveFilePath);
    QDir().mkpath(fileInfo.dir().absolutePath());

    QFile outFile(saveFilePath);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&outFile);
        for (const QString& line : linesToSave) {
            out << line << "\n";
        }
        outFile.close();
        QMessageBox::information(this, tr("Success"), tr("Gesture configuration saved to:\n%1").arg(saveFilePath));
        qDebug() << "GestureGuide: Successfully saved configuration to:" << saveFilePath;
        // REMOVED: Comment about notifying GestureProcessor
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not save gesture configuration to:\n%1\n\nError: %2").arg(saveFilePath).arg(outFile.errorString()));
        qWarning() << "GestureGuide: Failed to open file for writing:" << outFile.errorString();
    }
}
