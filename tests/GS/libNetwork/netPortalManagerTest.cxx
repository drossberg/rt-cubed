#include "GS/libNetwork/NetMsg.h"
#include "GS/libNetwork/NetMsgFactory.h"
#include "GS/libNetwork/GenericOneStringMsg.h"
#include "GS/libNetwork/GenericOneByteMsg.h"
#include "GS/libNetwork/GenericTwoBytesMsg.h"
#include "GS/libNetwork/GenericFourBytesMsg.h"
#include "GS/libNetwork/GenericMultiByteMsg.h"

#include "GS/libNetwork/FailureMsg.h"
#include "GS/libNetwork/SuccessMsg.h"

#include "GS/libNetwork/RemHostNameSetMsg.h"
#include "GS/libNetwork/NewHostOnNetMsg.h"

#include "GS/libNetwork/GeometryReqMsg.h"
#include "GS/libNetwork/GeometryManifestMsg.h"
#include "GS/libNetwork/GeometryChunkMsg.h"

#include "GS/libNetwork/NetPortalManager.h"
#include "GS/libNetwork/FailureMsg.h"

#include "NetPortalManagerTester.h"

#include <QCoreApplication>

/* 
 * =====================
 *
 *        Main
 *
 * =====================
 */

int main(int argc, char* argv[])
{
    //Setup
    QHostAddress netMan01Addy = QHostAddress::LocalHost;
    quint16 netMan01Port = 6000;

    QHostAddress netMan02Addy = QHostAddress::LocalHost;
    quint16 netMan02Port = 7000;

    //Core app
    QCoreApplication coreApp(argc, argv);

    NetPortalManager netMan01("Gomer");
    NetPortalManager netMan02("Kiaser");

    NetPortalManagerTester npmt01(&netMan01);
    NetPortalManagerTester npmt02(&netMan02);

    netMan01.listen(netMan01Addy, netMan01Port);
    netMan02.listen(netMan02Addy, netMan02Port);

    //Connect to netMan02
    NetPortal* np01 = npmt01.getNewPortal();
    NetPortal* np02 = npmt02.getNewPortal();
    np01->connectToHost(netMan02Addy, netMan02Port);

    np01->quickSend(12345677);

    return coreApp.exec();
}
