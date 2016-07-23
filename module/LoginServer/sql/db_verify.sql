/*
Navicat MySQL Data Transfer

Source Server         : 192.168.1.1
Source Server Version : 50611
Source Host           : 192.168.1.1:3306
Source Database       : db_verify

Target Server Type    : MYSQL
Target Server Version : 50611
File Encoding         : 65001

Date: 2016-07-23 15:23:56
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for account
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
  `id` int(4) unsigned NOT NULL AUTO_INCREMENT COMMENT '账号id',
  `user` char(32) NOT NULL COMMENT '用户名',
  `passwd` char(64) NOT NULL COMMENT '密码',
  `regip` char(16) DEFAULT NULL COMMENT '注册ip',
  `regtime` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '注册时间',
  `platform` char(16) DEFAULT NULL COMMENT '注册平台',
  `os` char(32) DEFAULT NULL COMMENT '注册系统',
  `model` char(32) DEFAULT NULL COMMENT '注册机型',
  `deviceid` char(64) DEFAULT NULL COMMENT '注册设备id',
  `visit` int(4) unsigned NOT NULL DEFAULT '0' COMMENT '访问次数',
  `lastip` char(16) DEFAULT NULL COMMENT '最后登录IP',
  `lastdevice` char(64) DEFAULT NULL COMMENT '最后登录设备id',
  `lasttime` timestamp NULL DEFAULT NULL COMMENT '最后登录时间',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '账号状态',
  PRIMARY KEY (`id`),
  UNIQUE KEY `account_idx_1` (`id`) USING BTREE,
  UNIQUE KEY `account_idx_2` (`user`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for blacklist
-- ----------------------------
DROP TABLE IF EXISTS `blacklist`;
CREATE TABLE `blacklist` (
  `ip` char(16) NOT NULL COMMENT '封禁ip',
  `cause` char(64) DEFAULT NULL COMMENT '封禁原因',
  `starttime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '开始封禁时间',
  `endtime` timestamp NULL DEFAULT NULL COMMENT '结束封禁时间',
  PRIMARY KEY (`ip`),
  UNIQUE KEY `blacklist_idx_1` (`ip`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for partition
-- ----------------------------
DROP TABLE IF EXISTS `partition`;
CREATE TABLE `partition` (
  `id` smallint(2) unsigned NOT NULL COMMENT '分区id',
  `name` char(64) NOT NULL COMMENT '分区名称',
  `status` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '分区状态(0正常, 1停机)',
  `recommend` tinyint(1) unsigned NOT NULL DEFAULT '0' COMMENT '是否是推荐分区',
  `createtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `partition_idx_1` (`id`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Procedure structure for sign_in
-- ----------------------------
DROP PROCEDURE IF EXISTS `sign_in`;
DELIMITER ;;
CREATE DEFINER=`root`@`%` PROCEDURE `sign_in`(IN `_user` char(32), IN `_passwd` char(64), IN `_ip` char(16), IN `_devid` char(64), OUT `_user_id` int(4))
BEGIN
  DECLARE exits INT DEFAULT NULL;
  DECLARE userid INT(4) DEFAULT NULL;
  DECLARE passwd CHAR(64) DEFAULT NULL;
  SELECT id, passwd FROM `account` WHERE `user`=_user INTO userid, passwd;
  # 用户是否存在
  IF ISNULL(userid) THEN
    SET `_user_id` = 0;
  # 密码是否正确
  ELSEIF passwd != _passwd THEN
    SET `_user_id` = 0;
  ELSE
    # 是否在黑名单
    SELECT COUNT(*) FROM `blacklist` WHERE `ip`=_ip AND CURRENT_TIMESTAMP() < `endtime` INTO exits;
    IF exits > 0 THEN
      SET `_user_id` = 0;
    ELSE
      # 更新访问信息
      UPDATE `account` SET `visit`=`visit`+1, `lastip`=_ip, `lastdevice`=_devid, `lasttime`=CURRENT_TIMESTAMP();
      SET `_user_id` = userid;
    END IF;
  END IF; 
END
;;
DELIMITER ;

-- ----------------------------
-- Procedure structure for sign_up
-- ----------------------------
DROP PROCEDURE IF EXISTS `sign_up`;
DELIMITER ;;
CREATE DEFINER=`root`@`%` PROCEDURE `sign_up`(IN `_user` char(32),IN `_passwd` char(64),IN `_regip` char(16),IN `_platform` char(16),IN `_os` char(32),IN `_model` char(32),IN `_devid` char(64),OUT `_user_id` int(4))
BEGIN
  # 是否在黑名单
  DECLARE exits INT DEFAULT NULL;
  SELECT COUNT(*) FROM `blacklist` WHERE `ip`=_regip AND CURRENT_TIMESTAMP() < `endtime` INTO exits;
  IF exits > 0 THEN
    SET _user_id = 0;
  ELSE
    # 用户名是否重复
    SELECT COUNT(*) FROM `account` WHERE `user`=_user INTO exits;
    IF exits > 0 THEN
      SET _user_id = 0;
    ELSE
      # 创建新用户
      INSERT INTO `account`(`user`,`passwd`,`regip`,`platform`,`os`,`model`,`deviceid`)
        VALUES(_user,_passwd,_regip,_platform,_os,_model,_devid);
      SET _user_id = LAST_INSERT_ID();
	  END IF;
  END IF;
END
;;
DELIMITER ;
