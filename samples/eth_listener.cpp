/*
 * Asio-Raw-LinkLayer - Boost.Asio raw link-layer socket.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file eth_listener.cpp
 * \brief Ethernet listener sample.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <cstdlib>
#include <cstddef>
#include <csignal>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <boost/array.hpp>

#include "ll_protocol.hpp"

/**
 * \var g_run
 * \brief Runing state of the program.
 */
static volatile bool g_run = true;

/**
 * \brief Signal handler.
 * \param signum signal number.
 */
static void signal_handler(int signum)
{
  switch(signum)
  {
    case SIGINT:
    case SIGTERM:
      g_run = false;
      break;
    default:
      break;
  }
}

/**
 * \brief Converts ethernet address to human-readable form.
 * \param src binary ethernet address.
 * \return std::string containing human-readable form or empty string if error.
 */
static std::string eth_ntop(void* src)
{
  uint8_t* s = reinterpret_cast<uint8_t*>(src);
  std::ostringstream oss;

  oss << std::setw(2) << std::setfill('0') << std::hex
    << static_cast<uint32_t>(s[0]);
  for(size_t i = 1 ; i < ETH_ALEN ; i++)
  {
    oss << ":" << std::setw(2) <<std::setfill('0') << std::hex
      << static_cast<uint32_t>(s[i]);
  }

  return oss.str();
}

/**
 * \brief Entry point of the program.
 * \param argc number of arguments.
 * \param argv array of arguments.
 * \return EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char** argv)
{
  std::array<char, 1500> buffer;
  struct sigaction sa;
  char* ifname = nullptr;

  if(argc > 1)
  {
    ifname = argv[1];
  }

  memset(&sa, 0x00, sizeof(struct sigaction));

  sa.sa_handler = signal_handler;
  sigfillset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if(sigaction(SIGINT, &sa, nullptr))
  {
    std::cerr << "Failed to catch SIGINT" << std::endl;
  }

  if(sigaction(SIGTERM, &sa, nullptr))
  {
    std::cerr << "Failed to catch SIGTERM" << std::endl;
  }

  try
  {
    using namespace asio::raw::ll;
    boost::asio::io_service ios;
    ll_endpoint<ll_protocol> endpoint;

    if(ifname)
    {
      endpoint = ll_endpoint<ll_protocol>(ifname, ETH_P_IP);
    }
    else
    {
      endpoint = ll_endpoint<ll_protocol>(ETH_P_IP);
    }

    ll_protocol::socket socket(ios, endpoint);
    socket.bind(endpoint);

    std::cout << "Raw socket running" << std::endl;

    while(g_run)
    {
      boost::system::error_code err;
      ll_protocol::endpoint remote;
      size_t ret = 0;
      struct ether_header* hdr = nullptr;

      ret = socket.receive_from(boost::asio::buffer(buffer), remote, 0,
          err);
      if(err && err != boost::asio::error::message_size)
      {
        std::cerr << "Error receiving: " << err << std::endl;
        throw boost::system::system_error(err);
      }

      if(ret < sizeof(struct ether_header))
      {
        // data too small
        continue;
      }

      hdr = reinterpret_cast<struct ether_header*>(buffer.data());

      // print out ethernet header information
      std::cout << "Packet received: type=0x" << std::hex
        << ntohs(hdr->ether_type) << std::dec << " "
        << "dst_addr="
        << eth_ntop(hdr->ether_dhost) << " "
        << "src_addr="
        << eth_ntop(hdr->ether_shost) << " "
        << std::endl;
    }
  }
  catch(std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  std::cout << "Exiting..." << std::endl;
  return EXIT_SUCCESS;
}

