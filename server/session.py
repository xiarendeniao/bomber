#encoding=utf-8
from twisted.python import log
from opcodes import opcodes
import player
import struct, logging

#包头长度
HEADER_LEN = 2
#命令长度
OPCODE_LEN = 2 

class Package(object):
    def __init__(self, cmd, data = None):
        self.read_cursor = 0
        self.cmd = cmd
        if data:
            assert isinstance(data, str)
            self.pkg_data = data
        else:
            self.pkg_data = ''
        
    def ResetReadCursor(self):
        self.read_cursor = 0
        
    def GetFullSize(self):
        return 4 + len(self.pkg_data)
        
    def GetFullData(self):
        return struct.pack('>H', len(self.pkg_data)+2) + struct.pack('<H', self.cmd) + self.pkg_data
        
    def __lshift__(self, arg):
        #对于字符串，用[str_length(uint16)]跟[字符串]串接表示
        #log.msg('pkg %s << %s' % (opcodes[self.cmd],str(arg)), logLevel = logging.DEBUG)
        if isinstance(arg, tuple):
            assert len(arg) > 1
            format = arg[0]
            assert isinstance(format, str) and len(format) > 0
            if format[0] not in ('>','<'):
                format = '<' + format
            else:
                if format[0] == '>':
                    format = '<' + format[1:]
            self.pkg_data += struct.pack(format, *(arg[1:]))
        else:
            assert False
        return self
        
    def __rshift__(self, format):
        assert isinstance(format, str) and len(format) > 0
        if format[0] not in ('>','<'):
            format = '<' + format
        else:
            if format[0] == '>':
                format = '<' + format[1:]       
        size = struct.calcsize(format)
        if size > len(self.pkg_data) - self.read_cursor:
            return None
        rt = struct.unpack_from(format, self.pkg_data, self.read_cursor)
        self.read_cursor += size
        return rt

class Session(object):
    def __init__(self, factory, connection):
        self.factory = factory
        self.connection = connection
        self.pkg_size = None
        self.pkg_data = ''
        self.player = player.Player(self)
        
    def IsConnected(self):
        '''
        检测是否连接
        '''
        return self.connection != None
        
    def LostConnection(self):
        '''
        断开连接
        '''
        self.connection = None
        self.player.LostConnection()
        
    def SendPackage(self, pkg):
        '''
        向客户端发送数据包
        '''
        assert isinstance(pkg, Package)
        if self.SendData(pkg.GetFullData()):
            log.msg('sent %s %s (%d bytes)' % (self.player.id,opcodes[pkg.cmd],pkg.GetFullSize()), logLevel = logging.DEBUG)
        
    def SendData(self, data):
        '''
        向客户端发送数据
        '''
        if not self.connection:
            log.msg('connection not found for player %s' % self.player.id, logLevel = logging.WARNING)
            return False
        else:
            self.connection.transport.write(data)
            return True
        
    def ReadData(self, data):
        '''
        从客户端接收数据
        '''
        self.pkg_data += data
        if self.pkg_size == None:
            #解析网络包长度
            if len(self.pkg_data) >= HEADER_LEN:
                (self.pkg_size, ) = struct.unpack('>H', self.pkg_data[0:HEADER_LEN])
                self.pkg_data = self.pkg_data[HEADER_LEN:]
        else:
            return
        #等待更多的数据
        if len(self.pkg_data) < self.pkg_size:
            return
        #解析命令
        opcode = struct.unpack('<H', self.pkg_data[0:OPCODE_LEN])[0]
        #执行对应的处理函数
        if opcode not in opcodes:
            log.msg('opcode %s unrecoginzed.' % opcode, logLevel = logging.WARNING)
        else:
            opcodeStr = opcodes[opcode]
            log.msg('received %s %s (%d bytes)' % (self.player.id,opcodeStr,2+len(self.pkg_data)))
            if not hasattr(self, opcodeStr):
                log.msg('opcode method "%s" not found' % opcodeStr, logLevel = logging.WARNING)
            else:
                getattr(self, opcodeStr)(Package(opcode, self.pkg_data[OPCODE_LEN:self.pkg_size]))
        #重置数据接收区域
        self.pkg_data = ''
        self.pkg_size = None
        
    def CMSG_ROOM_CREATE(self, pkg):
        (mapId,maxCount) = pkg >> 'HB'
        self.player.CreateRoom(mapId, maxCount)

    def CMSG_MOVE(self, pkg):
        (x,y,roleId,steps,dir) = pkg >> 'HHIBB'
        if self.player.role_id != roleId:
            log.msg('player %s (roleId %s) attemp to move role %s not owned' % (self.player.id, self.player.role_id, roleId), logLevel = logging.WARNING)
            return
        if not self.player.room:
            log.msg('player %s not in map attemp to move role %s' % (self.player.id, roleId), logLevel = logging.WARNING)
            return
        if not self.player.room.map:
            log.msg('player %s attemp to move role %s while map not running' % (self.player.id, roleId), logLevel = logging.WARNING)
            return
        self.player.room.map.HeroMove(x, y, roleId, steps, dir)
    
    def CMSG_ROOM_ENTER(self, pkg):
        (roomId, ) = pkg >> 'H'
        self.player.EnterRoom(roomId)
        
    def CMSG_ROOM_LEAVE(self, pkg):
        (roomId, ) = pkg >> 'H'
        self.player.LeaveRoom(roomId)
    
    def CMSG_ROOM_READY(self, pkg):
        (ready, ) = pkg >> 'H'
        #等待/取消等待的处理逻辑 markbyxds
         
    def CMSG_BOMB_BORN(self, pkg):
        (x, y) = pkg >> 'HH'
        #检验roleId是否合法
        if not self.player.role_id:
            log.msg('player %s with no role attemp to born bomb' % (self.player.id), logLevel = logging.WARNING)
            return
        self.player.room.map.BombBorn(self.player.role_id, x, y)
        
if __name__ == '__main__':
    pkg = Package(10)
    pkg << ('H', 10) << ('f', 12.5)
    print 'pkg.cmd = %d, len(pkg.pkg_data) = %d, pkg.pkg_data = %s' % (pkg.cmd, len(pkg.pkg_data), pkg.pkg_data)
    print 'len(pkg.GetFullData()) = %d, pkg.GetFullData() = %s' %(len(pkg.GetFullData()), pkg.GetFullData())
    data1,data2 = pkg >> 'Hf'
    print 'data1 = ', data1, ' data2 = ', data2, 'pkg.read_cursor = ', pkg.read_cursor