#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QResource>

#include "winsparkle.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(this, SIGNAL(windowWasShown()), this, SLOT(initWinSparkle()),
            Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    // showEvent() is called right *before* the window is shown, but WinSparkle
    // requires that the main UI of the application is already shown when
    // calling win_sparkle_init() (otherwise it could show its updates UI
    // behind the app instead of at top). By using a helper signal, we delay
    // calling initWinSparkle() until after the window was shown.
    //
    // Alternatively, one could achieve the same effect in arguably a simpler
    // way, by initializing WinSparkle in main(), right after showing the main
    // window. See https://github.com/vslavik/winsparkle/issues/41#issuecomment-70367197
    // for a discussion of this.
    emit windowWasShown();
}

void MainWindow::initWinSparkle()
{
    // Setup updates feed. This must be done before win_sparkle_init(), but
    // could be also, often more conveniently, done using a VERSIONINFO Windows
    // resource. See the "psdk" example and its .rc file for an example of that
    // (these calls wouldn't be needed then).
    win_sparkle_set_appcast_url("https://winsparkle.org/example/appcast.xml");
    win_sparkle_set_app_details(L"winsparkle.org", L"WinSparkle Qt Example", L"1.0");

    // Set DSA public key used to verify update's signature.
    // This is na example how to provide it from external source (i.e. from Qt
    // resource). See the "psdk" example and its .rc file for an example how to
    // provide the key using Windows resource.
    win_sparkle_set_dsa_pub_pem(reinterpret_cast<const char *>(QResource(":/pem/dsa_pub.pem").data()));

    // Initialize the updater and possibly show some UI
    win_sparkle_init();
}

void MainWindow::checkForUpdates()
{
    win_sparkle_check_update_with_ui();
}

MainWindow::~MainWindow()
{
    win_sparkle_cleanup();
    delete ui;
}
