#
#
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=gpio_drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/gpio_drv
  SUBMENU:=Other modules
  TITLE:=gpio_drv
  FILES:=$(PKG_BUILD_DIR)/gpio_drv.ko
  KCONFIG:=
endef

define KernelPackage/gpio_drv/description
  This is a gpio_drv drivers
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
	$(MAKE_OPTS) modules
endef

$(eval $(call KernelPackage,gpio_drv))
