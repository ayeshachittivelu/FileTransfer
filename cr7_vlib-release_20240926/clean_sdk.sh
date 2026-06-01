#!/bin/bash
# R-Car CR7 SDK CAN Driver Isolation Script

echo "Starting SDK cleanup for CAN driver analysis..."

# 1. Remove Heavy Applications (Graphics, Audio, Video, Network)
echo "Removing unrelated applications..."
rm -rf app/2d-wow-cluster-demo
rm -rf app/audio
rm -rf app/cluster_wrapper
rm -rf app/dcu_sample
rm -rf app/ether_sample
rm -rf app/fastboot_mono
rm -rf app/fastboot_qnxhv
rm -rf app/fatfs_emmc
rm -rf app/hello_rgl
rm -rf app/hello_rgl_wrapper
rm -rf app/imr_sample
rm -rf app/logo
rm -rf app/lwip_app
rm -rf app/mmc_sample
rm -rf app/rearview_camera
rm -rf app/rivp_sample
rm -rf app/rtp_player
rm -rf app/surround_view
rm -rf app/vspi_sample

# 2. Clean up Taurus Middleware (Keep CAN, remove Camera/Ethernet/RVGC)
echo "Pruning Taurus middleware..."
rm -rf app/taurus/driver/camera
rm -rf app/taurus/driver/eth
rm -rf app/taurus/driver/rvgc
rm -rf app/taurus/src/perserv/cameraserv
rm -rf app/taurus/src/perserv/ethserv
rm -rf app/taurus/src/perserv/rvgcserv

# 3. Clean up CIO Bridge (Keep CAN/CANFD, remove Ether/VIN/WM/Dummy)
echo "Pruning CIO layer..."
rm -rf app/cio/driver/dummy
rm -rf app/cio/driver/r-car_ether
rm -rf app/cio/driver/r-car_vin
rm -rf app/cio/driver/r-car_wm

# 4. Remove Unrelated Hardware Drivers (Keep CPG, PFC, SYSC, TICK, PRR, DELAY, IO, GPIO)
echo "Removing unrelated hardware drivers..."
rm -rf drivers/audio
rm -rf drivers/csi2
rm -rf drivers/dcu
rm -rf drivers/dmac
rm -rf drivers/doc
rm -rf drivers/du
rm -rf drivers/emmc
rm -rf drivers/ether
rm -rf drivers/hdmi
rm -rf drivers/i2c
rm -rf drivers/imr
rm -rf drivers/lvds
rm -rf drivers/vin
rm -rf drivers/vsp2

# 5. Remove Heavy Middleware (Graphics, Network, Video)
echo "Removing unrelated middleware..."
rm -rf middleware/davehd
rm -rf middleware/drw2d
rm -rf middleware/lwip
rm -rf middleware/rivp
rm -rf middleware/rivp_log
rm -rf middleware/WindowMng

# 6. Optional OS Cleanup: Remove Trampoline targets (Since we are using FreeRTOS)
echo "Removing Trampoline OS target files..."
find . -type d -name "trampoline_salvatorxs" -exec rm -rf {} +
find . -type d -name "trampoline" -exec rm -rf {} +

echo "Cleanup complete! The SDK is now isolated to CAN and System dependencies."
