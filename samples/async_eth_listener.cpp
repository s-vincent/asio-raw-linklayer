/*
 * Asio-Raw-LinkLayer - Boost.Asio raw link-layer socket.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file eth_listener.cpp
 * \brief Asynchronous ethernet listener sample.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <cstdlib>

#include <iostream>
#include <iomanip>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/ref.hpp>

#include "ll_protocol.hpp"
#include "async_raw_server.hpp"

using namespace asio::raw::ll;

/**
 * \class eth_listener
 * \brief Etherner frame listener.
 */
class eth_listener : public async_raw_server
{
  public:
    eth_listener(boost::asio::io_service& ios, const std::string& ifname,
        int protocol = ETH_P_ALL)
      : async_raw_server(ios, ifname, protocol)
    {
    }

    /**
     * \brief Receive callback.
     * \param error error value.
     * \param nb number of bytes transferred.
     */
    virtual void handle_recv(
        const boost::system::error_code& error, size_t nb)
    {
      const struct ether_header* hdr = nullptr;
      const char* data = nullptr;

      if(error && error != boost::asio::error::message_size)
      {
        std::cerr << "Error receiving: " << error << std::endl;
        async_recv();
        return;
      }

      if(nb < sizeof(struct ether_header))
      {
        // data too small
        async_recv();
        return;
      }

      data = buffer().data();
      hdr = reinterpret_cast<const struct ether_header*>(data);

      // print out ethernet header information
      std::cout << "Packet received: type=0x" << std::hex
        << ntohs(hdr->ether_type) << std::dec << " "
        << "dst_addr="
        << eth_ntop(hdr->ether_dhost) << " "
        << "src_addr="
        << eth_ntop(hdr->ether_shost) << " "
        << std::endl;

      // start again an asynchronous receive
      async_recv();
    }

    /**
     * \brief Send callback.
     * \param error error value.
     * \param nb number of bytes transferred.
     */
    virtual void handle_send(const boost::system::error_code& error,
        size_t nb)
    {
      if(error && error != boost::asio::error::message_size)
      {
        std::cerr << "Error sending packet: " << error << std::endl;
        return;
      }

      std::cout << "Send packet of " << nb << " bytes" << std::endl;
    }

  private:
    /**
     * \brief Converts ethernet address to human-readable form.
     * \param src binary ethernet address.
     * \return std::string containing human-readable form or empty string if error.
     */
    static std::string eth_ntop(const void* src)
    {
      const uint8_t* s = reinterpret_cast<const uint8_t*>(src);
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
};

/**
 * \brief Signal handler.
 * \param signum signal number.
 */
static void signal_handler(const boost::system::error_code& error, int signum,
    boost::asio::io_service& ios)
{
  if(!error)
  {
    switch(signum)
    {
      case SIGINT:
      case SIGTERM:
        ios.stop();
        break;
      default:
        break;
    }
  }
}

/**
 * \brief Entry point of the program.
 * \param argc number of arguments.
 * \param argv array of arguments.
 * \return EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char** argv)
{
  char* ifname = nullptr;

  if(argc > 1)
  {
    ifname = argv[1];
  }

  try
  {
    boost::asio::io_service ios;
    eth_listener server(ios, ifname ? ifname : "", ETH_P_ALL);

    // signals handling
    boost::asio::signal_set signals(ios, SIGINT, SIGTERM);
    signals.async_wait(boost::bind(signal_handler, _1, _2,
          boost::ref(ios)));

    std::cout << "Raw socket running" << std::endl;
    server.async_recv();

    /* send invalid packet */
    {
      char buf[32];

      buf[0] = 0x00;
      buf[1] = 0x01;
      buf[2] = 0x02;
      buf[3] = 0x03;
      buf[4] = 0x04;
      buf[5] = 0x05;
      buf[6] = 0x06;
      buf[7] = 0x07;
      buf[8] = 0x08;
      buf[9] = 0x09;
      buf[10] = 0x0A;
      buf[11] = 0x0B;
      buf[12] = static_cast<char>(0x86);
      buf[13] = static_cast<char>(0xDD);
      server.async_send(buf, 14);
    }

    ios.run();
  }
  catch(std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  std::cout << "Exiting..." << std::endl;
  return EXIT_SUCCESS;
}

