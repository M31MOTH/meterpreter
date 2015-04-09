/*!
 * @file remote.h
 * @brief Declarations of functions and types that interact with a remote endpoint.
 */
#ifndef _METERPRETER_LIB_REMOTE_H
#define _METERPRETER_LIB_REMOTE_H

#include "crypto.h"
#include "thread.h"

#ifdef _WIN32
typedef wchar_t* STRTYPE;
#else
typedef char* STRTYPE;
#endif

// Forward declarations required to keep compilers happy.
typedef struct _Packet Packet;
typedef struct _PacketRequestCompletion PacketRequestCompletion;
typedef struct _Transport Transport;
typedef struct _Remote Remote;

typedef SOCKET(*PTransportGetSocket)(Transport* transport);
typedef void(*PTransportReset)(Transport* transport);
typedef BOOL(*PTransportInit)(Remote* remote, SOCKET fd);
typedef BOOL(*PTransportDeinit)(Remote* remote);
typedef void(*PTransportDestroy)(Remote* remote);
typedef BOOL(*PServerDispatch)(Remote* remote, THREAD* dispatchThread);
typedef DWORD(*PPacketTransmit)(Remote* remote, Packet* packet, PacketRequestCompletion* completion);
typedef DWORD(*PPacketReceive)(Remote* remote, Packet** packet);

typedef struct _TcpTransportContext
{
	SOCKET fd;                            ///! Remote socket file descriptor.
	SSL_METHOD* meth;                     ///! The current SSL method in use.
	SSL_CTX* ctx;                         ///! SSL-specific context information.
	SSL* ssl;                             ///! Pointer to the SSL detail/version/etc.
	struct sockaddr_storage sock_desc;    ///! Details of the current socket.
	int sock_desc_size;                   ///! Details of the current socket.
	BOOL bound;                           ///! Flag to indicate if the socket was a bound socket.
} TcpTransportContext;

typedef struct _HttpTransportContext
{
	BOOL ssl;                             ///! Flag indicating whether the connection uses SSL.
	STRTYPE uri;                          ///! URI endpoint in use during HTTP or HTTPS transport use.
	HANDLE internet;                      ///! Handle to the internet module for use with HTTP and HTTPS.
	HANDLE connection;                    ///! Handle to the HTTP or HTTPS connection.
	unsigned char* cert_hash;             ///! Pointer to the 20-byte certificate hash to validate

	STRTYPE ua;                           ///! User agent string.
	STRTYPE proxy;                        ///! Proxy details.
	STRTYPE proxy_user;                   ///! Proxy username.
	STRTYPE proxy_pass;                   ///! Proxy password.
} HttpTransportContext;

typedef struct _Transport
{
	DWORD type;                           ///! The type of transport in use.
	PTransportGetSocket get_socket;       ///! Function to get the socket from the transport.
	PTransportReset transport_reset;      ///! Function to reset/clean the transport ready for restarting.
	PTransportInit transport_init;        ///! Initialises the transport.
	PTransportDeinit transport_deinit;    ///! Deinitialises the transport.
	PTransportDestroy transport_destroy;  ///! Destroy the transport.
	PServerDispatch server_dispatch;      ///! Transport dispatch function.
	PPacketTransmit packet_transmit;      ///! Transmits a packet over the transport.
	PPacketReceive packet_receive;        ///! Receives a packet over the transport.
	STRTYPE url;                          ///! Full URL describing the comms in use.
	VOID* ctx;                            ///! Pointer to the type-specific transport context;
	int expiration_time;                  ///! Number of seconds from starting to when the server should shut down.
	int expiration_end;                   ///! Unix timestamp for when the server should shut down.
	int start_time;                       ///! Unix timestamp representing the session startup time.
	int comms_last_packet;                ///! Unix timestamp of the last packet received.
	int comms_timeout;                    ///! Number of seconds to wait for a valid packet before timing out.
	DWORD retry_total;                    ///! Number of seconds to try to reestablish communications on failure.
	DWORD retry_wait;                     ///! Number of seconds to wait between each retry attempt.
} Transport;

/*!
 * @brief Remote context allocation.
 * @details Wraps the initialized file descriptor for extension purposes.
 *          A \c Remote is effectively a pointer to a remote client context
 *          which contains magic pixie dust that identifies the connection
 *          along with a way to interact with it.
 * @remark The `Original` and `Current` members are used to allow for
 *         functionality such as `rev2self` and reverting back to the initial
 *         desktop stations/desktops.
 */
typedef struct _Remote
{
	HMODULE hMetSrv;                      ///! Reference to the Meterpreter server instance.

	CryptoContext* crypto;                ///! Cryptographic context associated with the connection.

	Transport* transport;                 ///! Pointer to the currently used transport mechanism.
	Transport* nextTransport;             ///! Pointer to the next transport to use, if any.

	LOCK* lock;                           ///! General transport usage lock (used by SSL, and desktop stuff too).

	HANDLE hServerThread;                 ///! Handle to the current server thread.
	HANDLE hServerToken;                  ///! Handle to the current server security token.
	HANDLE hThreadToken;                  ///! Handle to the current thread security token.

	DWORD dwOrigSessionId;                ///! ID of the original Meterpreter session.
	DWORD dwCurrentSessionId;             ///! ID of the currently active session.
	char* cpOrigStationName;              ///! Original station name.
	char* cpCurrentStationName;           ///! Name of the current station.
	char* cpOrigDesktopName;              ///! Original desktop name.
	char* cpCurrentDesktopName;           ///! Name of the current desktop.
} Remote;

Remote* remote_allocate();
VOID remote_deallocate(Remote *remote);

VOID remote_set_fd(Remote *remote, SOCKET fd);

DWORD remote_set_cipher(Remote *remote, LPCSTR cipher, struct _Packet *initializer);
CryptoContext *remote_get_cipher(Remote *remote);

#endif
