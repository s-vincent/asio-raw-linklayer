/*
 * Asio-Raw-LinkLayer - Boost.Asio raw link-layer socket.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file ll_protocol.hpp
 * \brief Link-layer protocol for Boost.Asio socket.
 * \author Sebastien Vincent
 * \date 2017
 */

#ifndef ASIO_RAW_LL_LL_PROTOCOL
#define ASIO_RAW_LL_LL_PROTOCOL

#include <boost/asio.hpp>

#ifdef __linux__
#include <sys/socket.h>
#include <sys/types.h>

#include <net/if.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#else
#error "This library supports only GNU/Linux!"
#endif

/**
 * \namespace asio
 */
namespace asio
{
  /**
   * \namespace raw
   */
  namespace raw
  {
    /**
     * \namespace ll
     */
    namespace ll
    {
      /**
       * \class ll_endpoint
       * \brief Link-layer protocol endpoint.
       * \code
       *  using namespace asio::raw::ll;
       *
       *  std::string ifname = "eth0";
       *  boost::asio::io_service ios;
       *  ll_endpoint<ll_protocol> endpoint(ifname, ETH_P_ALL);
       *  asio::raw::ll::ll_protocol::socket socket(ios);
       *
       *  socket.bind(endpoint);
       *
       *  while(1)
       *  {
       *    boost::system::error_code err;
       *    ll_protocol::endpoint remote;
       *    size_t ret = 0;
       *
       *    ret = socket.receive_from(boost::asio::buffer(buffer), remote, 0, err);
       *    if(err && err != boost::asio::error::message_size)
       *    {
       *      throw boost::system::system_error(err);
       *    }
       *
       *    // do stuff with packet
       *    // ...
       *  }
       * \endcode
       * \note Code adapted from https://stackoverflow.com/a/26220262
       */
      template <typename Protocol>
      class ll_endpoint
      {
        public:
          /**
           * \brief The protocol type associated with the endpoint typedef.
           */
          typedef Protocol protocol_type;

          /**
           * \brief The socket address type typedef.
           */
          typedef boost::asio::detail::socket_addr_type data_type;

          /**
           * \brief Constructor.
           * \param eth_protocol network protocol to monitor (default all).
           */
          ll_endpoint(uint16_t eth_protocol = ETH_P_ALL)
            : m_protocol_type(eth_protocol)
          {
            m_sockaddr.sll_family = PF_PACKET;
            // protocol is already htons() in ll_protocol
            m_sockaddr.sll_protocol = m_protocol_type.protocol();
            m_sockaddr.sll_ifindex = 0;
            m_sockaddr.sll_hatype = 1;
          }

          /**
           * \brief Constructor.
           * \param ifname interface name.
           * \param eth_protocol specific network protocol to monitor.
           */
          ll_endpoint(const std::string& ifname,
                  uint16_t eth_protocol = ETH_P_ALL)
            : m_protocol_type(eth_protocol)
          {
            unsigned int ifindex = 0;

            if(!ifname.empty())
            {
              ifindex = if_nametoindex(ifname.c_str());

              if(ifindex == 0 && errno != 0)
              {
                std::string str = "network interface '" + ifname +
                  "' does not exist";
                throw std::runtime_error(str);
              }
            }

            m_sockaddr.sll_family = PF_PACKET;
            // protocol is already htons() in ll_protocol
            m_sockaddr.sll_protocol = m_protocol_type.protocol();
            m_sockaddr.sll_ifindex = ifindex;
            m_sockaddr.sll_hatype = 1;
          }

          /**
           * \brief Constructor.
           * \param addr socket address.
           */
          ll_endpoint(struct sockaddr_ll& addr)
            : m_protocol_type(ETH_P_ALL)
          {
            m_sockaddr = addr;
          }

          /**
           * \brief Assign from another endpoint.
           * \param other endpoint to assign.
           * \return the current object.
           */
          ll_endpoint& operator=(const ll_endpoint& other)
          {
            m_sockaddr = other.m_sockaddr;
            return *this;
          }

          /**
           * \brief Returns the protocol associated with the endpoint.
           * \return protocol associated with the endpoint.
           */
          protocol_type protocol() const
          {
            return m_protocol_type;
          }

          /**
           * \brief Returns the underlying endpoint in the native type.
           * \return the underlying endpoint in the native type.
           */
          data_type* data()
          {
            return reinterpret_cast<struct sockaddr*>(&m_sockaddr);
          }

          /**
           * \brief Returns the underlying endpoint in the native type.
           * \return the underlying endpoint in the native type.
           */
          const data_type* data() const
          {
            return reinterpret_cast<const struct sockaddr*>(&m_sockaddr);
          }

          /**
           * \brief Returns the size of the endpoint in the native type.
           * \return the size of the endpoint in the native type.
           */
          std::size_t size() const
          {
            return sizeof(m_sockaddr);
          }

          /**
           * \brief Sets the underlying size of the endpoint in the native type.
           * \param s new size.
           * \note this function does nothing.
           */
          void resize(std::size_t s)
          {
            // nothing we can do here
            (void)s;
          }

          /**
           * \brief Returns the capacity of the endpoint in the native type.
           * \return the capacity of the endpoint in the native type.
           */
          std::size_t capacity() const
          {
            return sizeof(m_sockaddr);
          }

          /**
           * \brief Compare endpoints for inequality.
           * \param e1 first endpoint to compare.
           * \param e2 second endpoint to compare.
           * \return true if first is equal to the second endpoint.
           */
          friend bool operator==(const ll_endpoint<Protocol>& e1,
              const ll_endpoint<Protocol>& e2)
          {
            return e1.m_sockaddr == e2.m_sockaddr;
          }

          /**
           * \brief Compare endpoints for inequality.
           * \param e1 first endpoint to compare.
           * \param e2 second endpoint to compare.
           * \return true if first is not equal to the second endpoint.
           */
          friend bool operator!=(const ll_endpoint<Protocol>& e1,
              const ll_endpoint<Protocol>& e2)
          {
            return !(e1.m_sockaddr == e2.m_sockaddr);
          }

          /**
           * \brief Compare endpoints for ordering.
           * \param e1 first endpoint to compare.
           * \param e2 second endpoint to compare.
           * \return true if first is lower than second endpoint.
           */
          friend bool operator<(const ll_endpoint<Protocol>& e1,
              const ll_endpoint<Protocol>& e2)
          {
            return e1.m_sockaddr < e2.m_sockaddr;
          }

          /**
           * \brief Compare endpoints for ordering.
           * \param e1 first endpoint to compare.
           * \param e2 second endpoint to compare.
           * \return true if first is greater than second endpoint.
           */
          friend bool operator>(const ll_endpoint<Protocol>& e1,
              const ll_endpoint<Protocol>& e2)
          {
            return e2.m_sockaddr < e1.m_sockaddr;
          }

          /**
           * \brief Compare endpoints for ordering.
           * \param e1 first endpoint to compare.
           * \param e2 second endpoint to compare.
           * \return true if first is lower than second endpoint.
           */
          friend bool operator<=(const ll_endpoint<Protocol>& e1,
              const ll_endpoint<Protocol>& e2)
          {
            return !(e2 < e1);
          }

          /**
           * \brief Compare endpoints for ordering.
           * \param e1 first endpoint to compare.
           * \param e2 second endpoint to compare.
           * \return true if first is greater or equal than second endpoint.
           */
          friend bool operator>=(const ll_endpoint<Protocol>& e1,
              const ll_endpoint<Protocol>& e2)
          {
            return !(e1 < e2);
          }

        private:
          /**
           * \brief Link-layer socket address.
           */
          sockaddr_ll m_sockaddr;

          /**
           * \brief Link-layer protocol.
           */
          protocol_type m_protocol_type;
      };

      /**
       * \class ll_protocol
       * \brief Link-layer protocol associated with a link-layer endpoint.
       */
      class ll_protocol
      {
        public:
          /**
           * \brief The link-layer socket typedef.
           */
          typedef boost::asio::basic_raw_socket<ll_protocol> socket;

          /**
           * \brief The link-layer endpoint typedef.
           */
          typedef ll_endpoint<ll_protocol> endpoint;

          /**
           * \brief Constructor.
           * \param eth_protocol protocol identifier.
           * \param af_family network family identifier.
           */
          explicit ll_protocol(uint16_t eth_protocol = ETH_P_ALL,
                  int af_family = PF_PACKET)
            : m_protocol(htons(eth_protocol)),
            m_family(af_family)
          {
          }

          /**
           * \brief Returns type of protocol.
           * \return type of protocol.
           */
          int type() const
          {
            return SOCK_RAW;
          }

          /**
           * \brief Returns identifier of the protocol.
           * \return identifier of the protocol.
           */
          uint16_t protocol() const
          {
            return m_protocol;
          }

          /**
           * \brief Returns identifier for the protocol family.
           * \returns identifier for the protocol family.
           */
          int family() const
          {
            return m_family;
          }

        private:
          /**
           * \brief Protocol identifier.
           */
          uint16_t m_protocol;

          /**
           * \brief Network family identifier.
           */
          int m_family;
      };
    } /* namespace ll */
  } /* namespace raw */
} /* namespace asio */

#endif /* ASIO_RAW_LL_LL_PROTOCOL */

