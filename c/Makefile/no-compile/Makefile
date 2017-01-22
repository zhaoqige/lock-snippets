# 6Harmonics Inc.
# by Qige: qige@6harmonics.com, qige.zhao@6wilink.com
# 2016.11.16
# 2016.11.21

include $(TOPDIR)/rules.mk

PKG_NAME:=gws-files
PKG_VERSION:=10.1
PKG_RELEASE:=43

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GWS Scripts
  MAINTAINER:=Qige Zhao <qige@6harmonics.com>
endef

define Package/$(PKG_NAME)/description
  Scripts/Files that provide basic controls
endef

define Package/$(PKG_NAME)/conffiles
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

