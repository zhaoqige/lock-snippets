# 6Harmonics Inc.
# by Qige: qige@6harmonics.com, qige.zhao@6wilink.com, 2016.07.14

include $(TOPDIR)/rules.mk

PKG_NAME:=rarp
PKG_VERSION:=6.0
PKG_RELEASE:=23

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=RARP Client
  MAINTAINER:=Qige Zhao <qige@6harmonics.com>
endef

define Package/$(PKG_NAME)/description
  Suitalbe for all 6Harmonics GWS Seiral Products
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))


