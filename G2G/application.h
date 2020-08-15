#pragma once



#include <QApplication>
#include <QSharedMemory>

class Application : public QApplication {
    Q_OBJECT

public:
    Application(int& argc, char** argv)
        : QApplication(argc, argv, true)
    {
        _singular = new QSharedMemory("SharedMemoryForOnlyOneInstanceOfYourBeautifulApplication", this);
    }
    ~Application()
    {
        if (_singular->isAttached()) {
            _singular->detach();
        }
    }

    bool lock()
    {
        if (_singular->attach(QSharedMemory::ReadOnly)) {
            _singular->detach();
            return false;
        }
        if (_singular->create(1)) {
            return true;
        }

        return false;
    }

private:
    QSharedMemory* _singular; // shared memory !! SINGLE ACCESS
};

