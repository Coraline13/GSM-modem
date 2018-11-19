#include "wiznet_print.h"
#include "wiznet.h"
#include <stdio.h>

void PrintPhyStatus(WIZNET_PHY_STATUS status)
{
	printf ("PHY STATUS: ");
	if (status.link_status == LINK_DOWN)
	{
		printf ("LINK DOWN\t");
	}
	else
	{
		printf ("LINK UP\t");
	}
	if (status.powerdown_status == POWERDOWN_DISABLE)
	{
		printf ("POWER DOWN DISABLE\n");
	}
	else
	{
		printf ("POWER DOWN ENABLE\n");
	}
}

void PrintIpAddress(WIZNET_IP_ADDRESS ip_address)
{
	printf ("%d.%d.%d.%d\n", ip_address.ip1, ip_address.ip2, ip_address.ip3, ip_address.ip4); 
}

void PrintHwAddress(WIZNET_HW_ADDRESS address)
{
	printf ("%02X:%02X:%02X:%02X:%02X:%02X\n", address.hw1, address.hw2, address.hw3, address.hw4, address.hw5, address.hw6); 
}

void PrintSocketStatus(WIZNET_SOCKET_STATUS status)
{
	switch (status)
	{
		case SOCK_CLOSED:
		{
			printf ("SOCK_CLOSED");
			break;
		}
		case SOCK_ARP:
		{
			printf ("SOCK_ARP");
			break;
		}
		case SOCK_INIT:
		{
			printf ("SOCK_INIT");
			break;
		}
		case SOCK_LISTEN:
		{
			printf ("SOCK_LISTEN");
			break;
		}
		case SOCK_SYNSENT:
		{
			printf ("SOCK_SYNSENT");
			break;
		}
		case SOCK_SYNRECV:
		{
			printf ("SOCK_SYNRECV");
			break;
		}
		case SOCK_ESTABLISHED:
		{
			printf ("SOCK_ESTABLISHED");
			break;
		}
		case SOCK_FIN_WAIT:
		{
			printf ("SOCK_FIN_WAIT");
			break;
		}
		case SOCK_CLOSING:
		{
			printf ("SOCK_CLOSING");
			break;
		}
		case SOCK_TIME_WAIT:
		{
			printf ("SOCK_TIME_WAIT");
			break;
		}
		case SOCK_CLOSE_WAIT:
		{
			printf ("SOCK_CLOSE_WAIT");
			break;
		}
		case SOCK_LAST_ACK:
		{
			printf ("SOCK_LAST_ACK");
			break;
		}
		case SOCK_UDP:
		{
			printf ("SOCK_UDP");
			break;
		}
		case SOCK_IPRAW:
		{
			printf ("SOCK_IPRAW");
			break;
		}
		case SOCK_MACRAW:
		{
			printf ("SOCK_MACRAW");
			break;
		}
		case SOCK_PPPOE:
		{
			printf ("SOCK_PPPOE");
			break;
		}
	}
}
