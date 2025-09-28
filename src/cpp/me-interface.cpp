#include "bio/me-interface.h"
#include <QDebug>

namespace BioMining {
namespace Bio {

MEAInterface::MEAInterface(QObject *parent) : QObject(parent)
{
    qDebug() << "MEAInterface constructor called.";
}

MEAInterface::~MEAInterface()
{
    qDebug() << "MEAInterface destructor called.";
}

bool MEAInterface::isConnected() const
{
    qDebug() << "MEAInterface::isConnected() called, returning false by default.";
    return false;
}

void MEAInterface::stopAcquisition()
{
    qDebug() << "MEAInterface::stopAcquisition() called.";
}

void MEAInterface::connectToDevice()
{
    qDebug() << "MEAInterface::connectToDevice() called.";
}

} // namespace Bio
} // namespace BioMining
