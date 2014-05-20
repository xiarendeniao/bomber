# -*- coding: UTF-8 -*-
#WARNING: 接收数据全部用字符串做连接的话会不会效率太低？每个字符串都得在内存构建新的PyStringObject对象 markbyxds
from twisted.internet.protocol import Factory, Protocol
from twisted.internet import reactor
from twisted.python import log
import sys, logging
import session, player

class GameProtocol(Protocol):
    def getId(self):
        return str(self.transport.getPeer())
    
    def connectionMade(self):
        log.msg("received connection %s" % self.getId(), logLevel = logging.DEBUG)
        self.factory.Addconnection(self)
        assert not hasattr(self, 'session')
        self.session = session.Session(self.factory, self)
        player.PlayerMgr().BroadcastRoomList()
        
    def connectionLost(self, reason):
        log.msg('lost conection %s' % self.getId(), logLevel = logging.DEBUG)
        self.session.LostConnection()
        self.factory.Delconnection(self)
        
    def dataReceived(self, data):
        #log.msg('received %s bytes' % len(data), logLevel = logging.DEBUG)
        self.session.ReadData(data)

class GameFactory(Factory):
    protocol = GameProtocol
    def __init__(self):
        self.__connections = list()
        
    def Addconnection(self, connection):
        self.__connections.append(connection)
        
    def Delconnection(self, connection):
        self.__connections.remove(connection)
        
    def SendAll(self, data):
        for connection in self.__connections:
            connection.transport.write(data)
            
def startlog():
    log.startLogging(sys.stdout)
    
reactor.callLater(0, startlog)
reactor.listenTCP(9999, GameFactory())
reactor.run()