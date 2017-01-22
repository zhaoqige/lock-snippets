# 6Harmonics Inc.
# by Qige: qige@6harmonics.com, qige.zhao@6wilink.com, 2016.11.16

include $(TOPDIR)/rules.mk

APP_NAME:=gws-bb
PKG_NAME:=gws-bb
PKG_VERSION:=10.0
PKG_RELEASE:=41

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GWS3000 Baseband
  MAINTAINER:=Qige Zhao <qige@6harmonics.com>
  DEPENDS:=+libiwinfo
endef

define Package/$(PKG_NAME)/description
  Beta version, depends on libiwinfo, use shm/files to share.
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/sbin

	$(INSTALL_CONF) $(PKG_BUILD_DIR)/$(APP_NAME).conf $(1)/etc/config/$(APP_NAME)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(APP_NAME).serv $(1)/etc/init.d/$(APP_NAME)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(APP_NAME) $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))


