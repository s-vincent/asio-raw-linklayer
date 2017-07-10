/*
 * Asio-Raw-LinkLayer - Boost.Asio raw link-layer socket.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file async_raw_server.hpp
 * \brief Raw socket server.
 * \author Sebastien Vincent
 * \date 2017
 */

#ifndef ASIO_RAW_LL_ASYNC_RAW_SERVER_HPP
#define ASIO_RAW_LL_ASYNC_RAW_SERVER_HPP

#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>

#include "ll_protocol.hpp"

namespace asio
{
  namespace raw
  {
    namespace ll
    {
      /**
       * \class async_raw_server
       * \brief Asynchronous raw link-layer server socket.
       */
      class async_raw_server : private boost::noncopyable
      {
        public:
          /**
           * \brief Constructor.
           * \param ios Boost.Asio IO service.
           * \param ifname interface or empty string to listen on all interface.
           * \param protocol network layer protocol number.
           */
          async_raw_server(boost::asio::io_service& ios,
                  const std::string& ifname,
              int protocol = ETH_P_ALL);

          /**
           * \brief Start receive operation.
           */
          void async_recv();

          /**
           * \brief Start send operation.
           * \param data data to send.
           */
          void async_send(const std::vector<char>& data);
          
          /**
           * \brief Start send operation.
           * \param data data to send.
           * \param data_len data length.
           */
          void async_send(const char* data, size_t data_len);

          /**
           * \brief Returns receive buffer.
           * \return buffer.
           */
          const std::array<char, 1500>& buffer() const;

        protected:
          /**
           * \brief Receive callback.
           * \param error error value.
           * \param nb number of bytes transferred.
           */
          virtual void handle_recv(const boost::system::error_code& error,
                  size_t nb) = 0;

          /**
           * \brief Send callback.
           * \param error error value.
           * \param nb number of bytes transferred.
           */
          virtual void handle_send(const boost::system::error_code& error,
                  size_t nb) = 0;

        private:
          /**
           * \brief Buffer for receive.
           */
          std::array<char, 1500> m_buffer;

          /**
           * \brief Link-layer endpoint.
           */
          asio::raw::ll::ll_protocol::endpoint m_endpoint; 

          /**
           * \brief Raw link-layer socket.
           */
          asio::raw::ll::ll_protocol::socket m_socket;
      };
    } /* namespace ll */
  } /* namespace raw */
} /* namespace asio */
#endif /* ASIO_RAW_LL_ASYNC_RAW_SERVER_HPP */

