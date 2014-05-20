#encoding=utf-8
from twisted.internet import reactor
from twisted.python import log
from opcodes import *
import tmxlib, logging, session

class Role(object):
	def __init__(self, roleId, x, y):
		self.id = roleId
		self.x = int(x)
		self.y = int(y)

class Map(object):
	def __init__(self, roomObj):
		self.id = roomObj.map_id
		self.tile_map = tmxlib.Map.open('resources/pk_%d.tmx' % self.id)
		self.room = roomObj
		self.bomb_dict = dict()
		#role_id map to Role Object 
		self.role_dict = dict()
		for ob in self.tile_map.layers['Objects1'].all_objects(): #ob: tmxlib.mapobject.MapObject
			if ob.name == 'man':
				roleId = int(ob.properties['roleId'])
				self.role_dict[roleId] = Role(roleId, ob.x, ob.y)
		
	def GetRoleIds(self):
		'''
		获取地图中的角色列表
		'''
		return self.role_dict.keys()
	
	def GetRoleCount(self):
		return len(self.role_dict)
	
	def CheckMoveable(self, x, y):
		if x < 0 or y < 0 or x >= self.tile_map.size[0] or y >= self.tile_map.size[1]:
			return False
		if self.tile_map.layers['Tiles1'][x,y].gid != 0:
			return False
		if (x,y) in self.bomb_dict:
			return False
		return True
	
	def HeroMove(self, x, y, roleId, steps, dir):
		'''
		dir:上下左右-->0123
		'''
		assert roleId in self.role_dict
		role = self.role_dict[roleId]
		#校验坐标
		if (role.x,role.y) != (x,y):
			log.msg('role(%d) position (%s,%s) not matched server (%s,%s)' % (roleId,x,y,role.x,role.y), logLevel = logging.WARNING)
			return False
		#校验路径
		newX, newY = x, y
		for step in range(1,steps+1):
			if dir == 0:
				newY -= 1
			elif dir == 1:
				newY += 1
			elif dir == 2:
				newX -= 1
			elif dir == 3:
				newX += 1
			else:
				log.msg('unrecognized dir %s' % dir, logLevel = logging.WARNING)
				return False
			if self.CheckMoveable(newX, newY):
				continue
			else:
				log.msg('(%s,%s) is not moveable' % (newX,newY), logLevel = logging.WARNING)
				return False
		#移动
		pkg = session.Package(SMSG_MOVE)
		pkg << ('HHIBB', x, y, roleId, steps, dir)
		self.room.BroadcastPackage(pkg)
		#修改位置
		role.x, role.y = newX, newY
		return True
		
	def BombBorn(self, roleId, x, y):
		assert roleId in self.role_dict
		#校验坐标
		role = self.role_dict[roleId]
		if (role.x, role.y) != (x,y):
			log.msg('role(%d) position (%s,%s) not matched server (%s,%s)' % (roleId,x,y,role.x,role.y), logLevel = logging.WARNING)
			return False
		#验证是否可以防止炸弹
		if (role.x,role.y) in self.bomb_dict:
			log.msg('role(%d) position (%s,%s) alread has bomb' % (roleId,x,y), logLevel = logging.WARNING)
			return False
		#通知客户端安置炸弹
		pkg = session.Package(SMSG_BOMB_BORN)
		pkg << ('HH', x, y)
		self.room.BroadcastPackage(pkg)
		#设置定时爆炸
		callId = reactor.callLater(5, self.BombExplode, x, y)
		#记录炸弹
		self.bomb_dict[(x,y)] = callId
		return True
	
	def BombExplode(self, x, y):
		#删除炸弹记录
		del self.bomb_dict[(x,y)]
		#通知客户单播放炸弹爆炸的动画
		pkg = session.Package(SMSG_BOMB_EXPLODE)
		pkg << ('HHB', x, y, 1)
		self.room.BroadcastPackage(pkg)
		
		#拆墙（不用通知客户端，客户端自己拆墙）
		posList = [(x-1,y), (x+1,y), (x,y-1), (x,y+1)]
		for pos in posList:
			tile = self.tile_map.layers['Tiles1'][pos[0],pos[1]]
			if tile.gid == 2:
				tile.gid = 0
		posList.append((x,y))
				
		#死人（需要通知客户端）
		#import pprint
		#pprint.pprint(self.role_dict)
		#pprint.pprint(self.posList)
		deadRoles = list()
		for role in self.role_dict.values():
			log.msg('role %d pos (%s,%s)' % (role.id,role.x,role.y), logLevel = logging.DEBUG)
			if (role.x,role.y) in posList:
				deadRoles.append(role)
		if len(deadRoles) > 0:
			pkg = session.Package(SMSG_ROLE_DEAD)
			data = ['B'+'H'*len(deadRoles), len(deadRoles)] + [deadRole.id for deadRole in deadRoles]
			pkg << tuple(data)
			self.room.BroadcastPackage(pkg)
		#删除死人
		for deadRole in deadRoles:
			del self.role_dict[deadRole.id]
			
		#判断游戏是否结束
		if len(self.role_dict) == 1:
			winRole = self.role_dict.values()[0]
			#停止炸弹的调度
			for pos,callId in self.bomb_dict.items():
				callId.cancel()
			log.msg('%s bombs cleared for game over' % len(self.bomb_dict), logLevel = logging.DEBUG)
			self.bomb_dict.clear()
			#game over package
			winPkg = session.Package(SMSG_GAME_OVER)
			winPkg << ('B', 1)
			losePkg = session.Package(SMSG_GAME_OVER)
			losePkg << ('B', 0)
			for playerObj in self.room.player_list:
				#通知客户端 game over
				if playerObj.role_id == winRole.id:
					playerObj.session.SendPackage(winPkg)
				else:
					playerObj.session.SendPackage(losePkg)
			#解散房间(地图)
			self.room.Dismiss()
			self.room = None