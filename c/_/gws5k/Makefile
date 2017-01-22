# 6Harmonics Inc.
# by Qige: qige@6harmonics.com, qige.zhao@6wilink.com, 2016.10.27/2016.11.09

include $(TOPDIR)/rules.mk

APP_NAME:=gws
PKG_NAME_5K:=gws5k
PKG_VERSION:=10.0
PKG_RELEASE:=37

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME_5K)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GWS5001 Local User Interface
  MAINTAINER:=Qige Zhao <qige@6harmonics.com>
  DEPENDS:=+gws5k-hw +gws-bb
endef

define Package/$(PKG_NAME_5K)/description
  Suitalbe for all 6Harmonics GWS5000 Seiral Products
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Package/$(PKG_NAME_5K)/install
	$(INSTALL_DIR) $(1)/usr/sbin

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(APP_NAME) $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME_5K)))


