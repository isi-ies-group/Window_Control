/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "nx_stm32_custom_driver.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
NX_PACKET_POOL pool_0;
NX_IP ip_0;

UCHAR pool_buffer[1536 * 24];
UCHAR ip_thread_stack[2048];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

   /* USER CODE BEGIN App_NetXDuo_MEM_POOL */
  (void)byte_pool;
  /* USER CODE END App_NetXDuo_MEM_POOL */
  /* USER CODE BEGIN 0 */
  nx_system_initialize();

  ret = nx_packet_pool_create(&pool_0,
                              "Main Packet Pool",
                              1536,
                              pool_buffer,
                              sizeof(pool_buffer));
  if(ret != NX_SUCCESS)
  {
      return ret;
  }


  ret = nx_ip_create(&ip_0,
                     "WiFi IP",
                     IP_ADDRESS(192,168,4,1),
                     0xFFFFFF00,
                     &pool_0,
                     nx_stm32_custom_driver,
                     ip_thread_stack,
                     sizeof(ip_thread_stack),
                     1);

  if(ret != NX_SUCCESS)
  {
      return ret;
  }

  static ULONG arp_cache_area[1024 / sizeof(ULONG)];

  ret = nx_arp_enable(&ip_0,
                      (void *)arp_cache_area,
                      sizeof(arp_cache_area));


  if(ret != NX_SUCCESS)
  {
      return ret;
  }


  ret = nx_icmp_enable(&ip_0);

  if(ret != NX_SUCCESS)
  {
      return ret;
  }

  ret = nx_tcp_enable(&ip_0);

  if(ret != NX_SUCCESS)
  {
      return ret;
  }

  ret = nx_udp_enable(&ip_0);

  if(ret != NX_SUCCESS)
  {
      return ret;
  }

  /* USER CODE END 0 */

  /* USER CODE BEGIN MX_NetXDuo_Init */
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
