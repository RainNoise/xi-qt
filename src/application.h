#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>


namespace xi {

class Application : public QApplication
{
	Q_OBJECT

public:
	Application(int &argc, char **argv);
	~Application();
};

}

#endif // APPLICATION_H
