#encoding=utf-8
from twisted.python import log
from twisted.internet import threads
import logging
from opcodes import *
import player, session, map

WAITING = 1
MAP_LOADING = 2
GAME_RUNNING = 3

class Room(object):
    MAX_ROOM_ID = 1
    def __init__(self, mapId, maxCount):
        self.status = WAITING
        self.map_id = mapId
        self.map = None
        self.max_player_count = maxCount
        self.id = self.__class__.MAX_ROOM_ID
        self.__class__.MAX_ROOM_ID += 1
        self.player_list = list()
        
    def StartGame(self):
        assert len(self.player_list) > 0
        #检测是否可以开始游戏
        for playerObj in self.player_list:
            if not playerObj.ready:
                return False
        #加载地图
        self.LoadMap()
        
    def LoadMap(self):
        self.status = MAP_LOADING
        d = threads.deferToThread(self.__loadmap)
        d.addCallbacks(self.__loadmap_success, self.__loadmap_fail)
            
    def __loadmap(self):
        '''
        加载地图，在独立线程中运行，成功则返回地图对象
        '''
        return map.Map(self)
    
    def __loadmap_success(self, mapObj):
        self.map = mapObj
        self.status = GAME_RUNNING
        #通知玩家进入游戏
        assert self.map.GetRoleCount() == len(self.player_list) #这个断言很可能会被触发，房间最大容纳的人数应该根据tmx地图中的信息来确定，那么tmx信息读取如何不阻塞呢？ markbyxds 
        playerIndex = 0
        for roleId in self.map.GetRoleIds():
            pkg = session.Package(SMSG_MAP_ENTER)
            pkg << ('HH', self.map_id, roleId)
            playerObj = self.player_list[playerIndex]
            playerObj.role_id = roleId
            playerObj.session.SendPackage(pkg)
            playerIndex += 1
    
    def __loadmap_fail(self, failure):
        log.msg('load map failed: %s' % str(failure), logLevel = logging.ERROR)
        self.status = WAITING
        #通知客户端离开房间
        pkg = session.Package(SMSG_ROOM_LEAVE)
        pkg << ('H', self.id)
        self.BroadcastPackage(pkg)
        #修改玩家身上的房间信息，并把玩家从列表中删除
        while len(self.player_list) > 0:
            playerObj = self.player_list.pop(0)
            playerObj.room = None
        #房间删除
        player.PlayerMgr().DelRoom(self)
        #广播房间列表
        player.PlayerMgr().BroadcastRoomList()
        
    def BroadcastPackage(self, pkg):
        for playerObj in self.player_list:
            playerObj.session.SendPackage(pkg)
            
    def BroadcastRoomInfo(self):
        '''
        广播房间详细信息给房间内的玩家
        '''
        if len(self.player_list) > 0:
            #构建数据包
            pkg = session.Package(SMSG_ROOM_INFO)
            pkg << ('HHBB', self.id, self.map_id, self.max_player_count, len(self.player_list))
            for playerObj in self.player_list:
                pkg << ('IH%dsB'%len(playerObj.name), playerObj.id, len(playerObj.name), playerObj.name, playerObj.ready)
            #广播数据包
            self.BroadcastPackage(pkg)
    
    def IsEmpty(self):
        return len(self.player_list) == 0
    
    def IsFull(self):
        return len(self.player_list) >= self.max_player_count
    
    def IsWaiting(self):
        return self.status == WAITING
    
    def HasPlayer(self, playerObj):
        return playerObj in self.player_list
    
    def PlayerEnter(self, playerObj):
        assert playerObj not in self.player_list
        self.player_list.append(playerObj)
        playerObj.room = self
        #通知客户端
        self.BroadcastRoomInfo()
        
    def PlayerLeave(self, playerObj):
        assert playerObj in self.player_list
        self.player_list.remove(playerObj)
        playerObj.room = None
        #通知客户端离开房间
        pkg = session.Package(SMSG_ROOM_LEAVE)
        pkg << ('H', self.id)
        playerObj.session.SendPackage(pkg)
        #通知剩余玩家更新房间信息
        self.BroadcastRoomInfo()
        #检测房间是否需要删除
        if self.IsEmpty():
            player.PlayerMgr().DelRoom(self)
        #通知客户端
        player.PlayerMgr().BroadcastRoomList()
        
    def Dismiss(self):
        '''
        解散房间(地图)
        '''
        while len(self.player_list) > 0:
            playerObj = self.player_list.pop()
            playerObj.room = None
            playerObj.role_id = 0
        self.map = None
        player.PlayerMgr().DelRoom(self)