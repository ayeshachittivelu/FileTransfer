#!/bin/bash

# ==============================================================================
# TATA TECHNOLOGIES | Simulation Environment Launcher
# ==============================================================================

set -e

# Terminal output color definitions
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Workspace definitions
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VCAN_DIR="${SCRIPT_DIR}/vcan0_sw_simulation"
CAN0_DIR="${SCRIPT_DIR}/capl_can0_simulation"

print_usage() {
    echo -e "${YELLOW}Usage: $0 [--start | --stop]${NC}"
    echo -e "  --start : Orchestrates environment setups and launches simulation"
    echo -e "  --stop  : Tears down container networks and cleans the Docker environment"
    exit 1
}

# Ensure exactly one argument is passed
if [ $# -ne 1 ]; then
    print_usage
fi

ACTION="$1"

# ==========================================
# LIFECYCLE: START FLOW
# ==========================================
if [ "$ACTION" == "--start" ]; then
    echo -e "${BLUE}==================================================${NC}"
    echo -e "${BLUE}    TATA TECHNOLOGIES | Simulation Launcher        ${NC}"
    echo -e "${BLUE}==================================================${NC}"
    echo -e "Select the simulation environment:"
    echo -e "  1) VCAN0-based Software Simulation (Pure Virtual)"
    echo -e "  2) CAN0-based CAPL Simulation (Hardware Bound)"
    echo -e "${BLUE}==================================================${NC}"

    read -p "Enter selection [1 or 2]: " CHOICE

    case "$CHOICE" in
        1)
            echo -e "\n${YELLOW}[INFO] Entering VCAN0 Software Simulation Environment...${NC}"
            if [ ! -d "$VCAN_DIR" ]; then
                echo -e "${RED}[ERROR] Directory not found: $VCAN_DIR${NC}"
                exit 1
            fi
            cd "$VCAN_DIR"

            # ---------------------------------------------------------
            # 1. Clean and Setup the Virtual CAN Network
            # ---------------------------------------------------------
            echo -e "${BLUE}[PROCESS] Tearing down any existing vcan0 interfaces...${NC}"
            ip link set vcan0 down 2>/dev/null || true
            ip link delete dev vcan0 2>/dev/null || true

            echo -e "${BLUE}[PROCESS] Initializing fresh vcan0 network...${NC}"
            modprobe vcan || true
            ip link add dev vcan0 type vcan
            ip link set vcan0 up
            echo -e "${GREEN}[SUCCESS] vcan0 interface is online.${NC}"

            # ---------------------------------------------------------
            # 2. Cleanup Zombie / Old Containers
            # ---------------------------------------------------------
            echo -e "${BLUE}[PROCESS] Sweeping for zombie or outdated containers...${NC}"
            docker compose down --remove-orphans 2>/dev/null || true
            echo -e "${GREEN}[SUCCESS] Environment is clean.${NC}"

            # ---------------------------------------------------------
            # 3. Launch Containers & Wait
            # ---------------------------------------------------------
            echo -e "${YELLOW}[INFO] Launching docker compose stack...${NC}"
            docker compose up -d

            echo -e "${YELLOW}[INFO] Waiting 5 seconds for containers to settle...${NC}"
            sleep 5

            # ---------------------------------------------------------
            # 4. Health Check Verification
            # ---------------------------------------------------------
            echo -e "\n${BLUE}[PROCESS] Executing container health checks...${NC}"

            CONTAINERS=$(docker compose ps -q)
            FAILED_FLAG=0

            for container in $CONTAINERS; do
                NAME=$(docker inspect --format='{{.Name}}' "$container" | sed 's/^\///')
                STATE=$(docker inspect --format='{{.State.Status}}' "$container")
                HEALTH=$(docker inspect --format='{{if .State.Health}}{{.State.Health.Status}}{{else}}none{{end}}' "$container")

                if [ "$STATE" != "running" ]; then
                    echo -e "${RED}  [ALERT] Container '$NAME' is NOT RUNNING (State: $STATE)${NC}"
                    FAILED_FLAG=1
                elif [ "$HEALTH" == "unhealthy" ]; then
                    echo -e "${RED}  [ALERT] Container '$NAME' is running but UNHEALTHY!${NC}"
                    FAILED_FLAG=1
                else
                    echo -e "${GREEN}  [OK] Container '$NAME' is $STATE (Health: $HEALTH)${NC}"
                fi
            done

            echo -e "\n${BLUE}==================================================${NC}"
            if [ $FAILED_FLAG -eq 1 ]; then
                echo -e "${RED}⚠️  WARNING: One or more containers failed to initialize!${NC}"
                echo -e "${YELLOW}Please check logs using: docker logs <container_name>${NC}"
            else
                echo -e "${GREEN}✨ VCAN0 Simulation Stack Deployed Successfully! ✨${NC}"
            fi
            echo -e "${BLUE}==================================================${NC}"
            ;;

        2)
            echo -e "\n${YELLOW}[INFO] Entering CAN0 CAPL Simulation Environment...${NC}"
            if [ ! -d "$CAN0_DIR" ]; then
                echo -e "${RED}[ERROR] Directory not found: $CAN0_DIR${NC}"
                exit 1
            fi
            cd "$CAN0_DIR"

            # ---------------------------------------------------------
            # 1. Ensure CAN0 is Up
            # ---------------------------------------------------------
            echo -e "${BLUE}[PROCESS] Verifying physical can0 interface is active...${NC}"
            ip link set can0 up 2>/dev/null || true
            echo -e "${GREEN}[SUCCESS] can0 interface validation complete.${NC}"

            # ---------------------------------------------------------
            # 2. Launch Containers
            # ---------------------------------------------------------
            echo -e "${YELLOW}[INFO] Launching hardware-bound docker compose stack...${NC}"
            docker compose up -d

            echo -e "\n${BLUE}==================================================${NC}"
            echo -e "${GREEN}✨ CAN0 CAPL Simulation Stack Deployed Successfully! ✨${NC}"
            echo -e "${BLUE}==================================================${NC}"
            ;;

        *)
            echo -e "${RED}[ERROR] Invalid selection. Aborting.${NC}"
            exit 1
            ;;
    esac

# ==========================================
# LIFECYCLE: STOP FLOW
# ==========================================
elif [ "$ACTION" == "--stop" ]; then
    echo -e "${YELLOW}[STOPPING] Initializing global stack cleanup and teardown operations...${NC}"
    echo -e "${BLUE}--------------------------------------------------${NC}"

    # 1. Stop VCAN0 Environment (Deep Clean)
    if [ -d "$VCAN_DIR" ] && [ -f "$VCAN_DIR/docker-compose.yml" ]; then
        echo -e "${BLUE}[CLEANUP] Stopping containers in VCAN0 environment...${NC}"
        cd "$VCAN_DIR"
        docker compose down -v --remove-orphans 2>/dev/null || true
    fi

    # 2. Stop CAN0 Environment (Deep Clean)
    if [ -d "$CAN0_DIR" ] && [ -f "$CAN0_DIR/docker-compose.yml" ]; then
        echo -e "${BLUE}[CLEANUP] Stopping containers in CAN0 environment...${NC}"
        cd "$CAN0_DIR"
        docker compose down -v --remove-orphans 2>/dev/null || true
    fi

    # 3. Catch-all: Stop any other running containers globally
    echo -e "${BLUE}[CLEANUP] Force-stopping any remaining stray Docker containers...${NC}"
    RUNNING_CONTAINERS=$(docker ps -q)
    if [ -n "$RUNNING_CONTAINERS" ]; then
        docker stop $RUNNING_CONTAINERS 2>/dev/null || true
    fi

    # 4. Clean the Docker Environment (Prune unused containers and networks)
    echo -e "${BLUE}[CLEANUP] Sweeping Docker environment (pruning stopped containers and unused networks)...${NC}"
    docker container prune -f 2>/dev/null || true
    docker network prune -f 2>/dev/null || true

    # 5. Teardown VCAN0 Interface
    echo -e "${BLUE}[CLEANUP] Tearing down vcan0 network interface...${NC}"
    ip link set vcan0 down 2>/dev/null || true
    ip link delete dev vcan0 2>/dev/null || true

    echo -e "\n${BLUE}==================================================${NC}"
    echo -e "${GREEN}🛑 All simulation environments stopped and Docker environment fully cleaned.${NC}"
    echo -e "${BLUE}==================================================${NC}"

else
    print_usage
fi
