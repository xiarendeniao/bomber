#encoding=utf-8
from twisted.python import log
from opcodes import *
import session, room
import logging

class Singleton(type):
    _instances = {}
    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:  
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)  
        return cls._instances[cls]

class PlayerMgr(object):
    __metaclass__ = Singleton
    def __init__(self):
        self.player_dict = dict()
        self.player_list = list()
        self.rooms = dict()
        
    def BroadcastRoomList(self):
        '''
        广播房间列表给还没进入地图的玩家
        '''
        #构建数据包
        pkg = session.Package(SMSG_ROOM_LIST)
        pkg << ('H', len(self.rooms))
        for roomObj in self.rooms.values():
            pkg << ('HBB', roomObj.id, roomObj.max_player_count, len(roomObj.player_list))
        #对还没进入地图的所有人广播房间列表
        for playerObj in self.player_list:
            if not playerObj.room or playerObj.room.IsWaiting():
                playerObj.session.SendPackage(pkg)

    def AddRoom(self, roomObj):
        assert roomObj.id not in self.rooms
        self.rooms[roomObj.id] = roomObj
        self.BroadcastRoomList()
        
    def DelRoom(self, roomObj):
        if roomObj.id in self.rooms:
            del self.rooms[roomObj.id]
        else:
            log.msg('room %d not found in mgr' % roomObj.id, logLevel = logging.WARNING)
        
    def AddPlayer(self, player):
        assert player.id not in self.player_dict
        assert player not in self.player_list
        self.player_dict[player.id] = player
        self.player_list.append(player)
        
    def DelPlayer(self, player):
        assert player.id in self.player_dict
        assert player in self.player_list
        del self.player_dict[player.id]
        self.player_list.remove(player)
        
    def GetPlayerList(self):
        return self.player_list

class Player(object):
    MAX_PLAYER_ID = 1
    def __init__(self, session_obj, playerId = None):
        self.session = session_obj
        if not playerId:
            self.id = self.__class__.MAX_PLAYER_ID
            self.__class__.MAX_PLAYER_ID += 1
        else:
            self.id = playerId
        self.name = 'player-%d' % self.id
        self.room = None
        self.role_id = 0
        self.ready = 1
        PlayerMgr().AddPlayer(self)

    def LostConnection(self):
        #从房间离开
        if self.room:
            self.room.PlayerLeave(self)
        #从玩家列表删除
        PlayerMgr().DelPlayer(self)

    def IsConnected(self):
        return self.session.IsConnected()
    
    def CreateRoom(self, mapId, maxCount):
        #判断是否可以创建房间
        if self.room:
            log.msg('player %s in room %s can not create new room' % (self.id, self.room.id), logLevel = logging.WARNING)
            return False
        #构建房间对象
        roomObj = room.Room(mapId, maxCount)
        #进入房间
        roomObj.PlayerEnter(self)
        #把房间加入管理器
        PlayerMgr().AddRoom(roomObj)

    def LeaveRoom(self, roomObj):
        #参数转换
        if not isinstance(roomObj, room.Room):
            assert isinstance(roomObj, (int,long))
            if roomObj not in PlayerMgr().rooms:
                log.msg('player %s attemp to leave room %s not exists' % (self.id,roomObj), logLevel = logging.WARNING)
                return False
            else:
                roomObj = PlayerMgr().rooms[roomObj]
        #房间判断
        if self.room != roomObj:
            log.msg('player %s not in room %d' % (self.id,roomObj.id), logLevel = logging.WARNING)
            return False
        if not roomObj.IsWaiting():
            log.msg('player %s leave running room %d failed' % (self.id,roomObj.id), logLevel = logging.WARNING)
            return False
        assert roomObj.HasPlayer(self)
        roomObj.PlayerLeave(self)
        return True
        
    def EnterRoom(self, roomId):
        #检查房间id的合法性
        if roomId not in PlayerMgr().rooms:
            log.msg('player attempt to enter room %s not exists' % (self.id, roomId), logLevel = logging.WARNING)
            return False
        else:
            newRoom = PlayerMgr().rooms[roomId]
            #检测房间是否已开始游戏
            if not newRoom.IsWaiting():
                log.msg('player %s attempt to enter running room %s' % (self.id, roomId), logLevel = logging.WARNING)
                return False
            #目标房间是否已满
            if newRoom.IsFull():
                log.msg('player %s attempt to enter full room %s' % (self.id, roomId), logLevel = logging.WARNING)
                return False
            if self.room:
                #先从当前房间离开
                if self.room.id == roomId:
                    log.msg('player %d alread entered room %d' % (self.id, self.room.id), logLevel = logging.WARNING)
                    return True
                else:
                    if not self.LeaveRoom(self.room):
                        return False
        #进入新的房间
        newRoom.PlayerEnter(self)
        #检测游戏是否可以开始了
        self.room.StartGame()
        return True