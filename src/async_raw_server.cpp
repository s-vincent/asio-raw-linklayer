/**
 * \file async_raw_server.cpp
 * \brief Raw socket server.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <memory>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "async_raw_server.hpp"

namespace asio
{
  namespace raw
  {
    namespace ll
    {
      async_raw_server::async_raw_server(boost::asio::io_service& ios,
          const std::string& ifname, int protocol)
        : m_endpoint(ifname, protocol),
        m_socket(ios, m_endpoint)
      {
      }

      void async_raw_server::async_recv()
      {
        asio::raw::ll::ll_protocol::endpoint remote;

        m_socket.async_receive_from(boost::asio::buffer(m_buffer), remote,
            boost::bind(&async_raw_server::handle_recv, this,
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred));
      }

      void async_raw_server::async_send(const std::vector<char>& data)
      {
        asio::raw::ll::ll_protocol::endpoint remote;
        const struct ether_header* hdr =
          reinterpret_cast<const struct ether_header*>(data.data());
        struct sockaddr_ll addr = *reinterpret_cast<struct sockaddr_ll*>(
            m_endpoint.data());

        addr.sll_halen = ETH_ALEN;
        memcpy(&addr.sll_addr, hdr->ether_dhost, ETH_ALEN);

        remote = asio::raw::ll::ll_protocol::endpoint(addr);

        // be sure to hold data lifetime in memory until send finished
        std::shared_ptr<std::vector<char>> copy =
          std::make_shared<std::vector<char>>(data);

        m_socket.async_send_to(boost::asio::buffer(*copy), remote,
            boost::bind(&async_raw_server::handle_send, this,
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred));
      }

      void async_raw_server::async_send(const char* data, size_t data_len)
      {
        std::vector<char> copy(data, data + data_len);

        async_send(copy);
      }

      const std::array<char, 1500>& async_raw_server::buffer() const
      {
        return m_buffer;
      }
    } /* namespace ll */
  } /* namespace raw */
} /* namespace asio */

