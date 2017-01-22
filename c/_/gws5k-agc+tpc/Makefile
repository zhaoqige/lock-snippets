# 6Harmonics Inc.
# by Qige: qige@6harmonics.com, qige.zhao@6wilink.com, 2016.11.16

include $(TOPDIR)/rules.mk

APP_NAME:=gws-tpc
PKG_NAME:=gws5k-tpc
PKG_VERSION:=10.0
PKG_RELEASE:=93

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)-38
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=8Watt for GWS5000 TxPower Control
  MAINTAINER:=Qige Zhao <qige@6harmonics.com>
  DEPENDS:=+libiwinfo +gws5k-hw +rarp +wrarpd
endef

define Package/$(PKG_NAME)-33
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=2Watt for GWS5000 TxPower Control
  MAINTAINER:=Qige Zhao <qige@6harmonics.com>
  DEPENDS:=+libiwinfo +gws5k-hw +rarp +wrarpd
endef

define Package/$(PKG_NAME)-38/description
  GWS5000 Max Txpower 8Watt
endef

define Package/$(PKG_NAME)-33/description
  GWS5000 Max Txpower 2Watt
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Package/$(PKG_NAME)-38/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d

	$(INSTALL_CONF) $(PKG_BUILD_DIR)/hw-38.txt $(1)/etc/hw.txt
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/$(APP_NAME)-38.conf $(1)/etc/config/$(APP_NAME)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(APP_NAME)-38 $(1)/usr/sbin/$(APP_NAME)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(APP_NAME).serv $(1)/etc/init.d/$(APP_NAME)
endef

define Package/$(PKG_NAME)-33/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d

	$(INSTALL_CONF) $(PKG_BUILD_DIR)/hw-33.txt $(1)/etc/hw.txt
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/$(APP_NAME)-33.conf $(1)/etc/config/$(APP_NAME)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(APP_NAME)-33 $(1)/usr/sbin/$(APP_NAME)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(APP_NAME).serv $(1)/etc/init.d/$(APP_NAME)
endef

$(eval $(call BuildPackage,$(PKG_NAME)-38))
$(eval $(call BuildPackage,$(PKG_NAME)-33))


