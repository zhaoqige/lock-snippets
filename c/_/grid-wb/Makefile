# 6Harmonics Inc.
# by Qige: qige@6harmonics.com, qige.zhao@6wilink.com, 2016.11.09

include $(TOPDIR)/rules.mk

PKG_NAME:=grid-wb
PKG_VERSION:=10.0
PKG_RELEASE:=33

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GWS Remote Interface Daemon, Lite version
  MAINTAINER:=Qige Zhao <qige@6harmonics.com>
  DEPENDS:=+libiwinfo +gws-bb
endef

define Package/$(PKG_NAME)/description
  LCD Controller Protocol v1.6c.091116 (+wb).
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

	$(INSTALL_CONF) $(PKG_BUILD_DIR)/$(PKG_NAME).conf $(1)/etc/config/$(PKG_NAME)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME).serv $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))


