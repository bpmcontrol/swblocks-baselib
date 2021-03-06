/*
 * This file is part of the swblocks-baselib library.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __UTEST_TESTMESSAGINGUTILS_H_
#define __UTEST_TESTMESSAGINGUTILS_H_

#include <utests/baselib/DataModelTestUtils.h>
#include <utests/baselib/LoggerUtils.h>
#include <utests/baselib/TestUtils.h>
#include <utests/baselib/TestTaskUtils.h>
#include <utests/baselib/MachineGlobalTestLock.h>

#include <baselib/messaging/ProxyBrokerBackendProcessingFactory.h>
#include <baselib/messaging/asyncrpc/ConversationProcessingBaseImpl.h>
#include <baselib/messaging/asyncrpc/ConversationProcessingTask.h>
#include <baselib/messaging/MessagingUtils.h>
#include <baselib/messaging/BrokerBackendProcessing.h>
#include <baselib/messaging/MessagingClientObjectDispatch.h>
#include <baselib/messaging/MessagingClientObject.h>
#include <baselib/messaging/BrokerFacade.h>
#include <baselib/messaging/MessagingClientFactory.h>
#include <baselib/messaging/MessagingClientBlockDispatchLocal.h>
#include <baselib/messaging/TcpBlockTransferServer.h>
#include <baselib/messaging/BrokerErrorCodes.h>

#include <baselib/tasks/Algorithms.h>
#include <baselib/tasks/TasksUtils.h>

#include <baselib/security/AuthorizationCacheImpl.h>
#include <baselib/security/AuthorizationServiceRest.h>
#include <baselib/security/SecurityInterfaces.h>

#include <baselib/core/ObjModel.h>
#include <baselib/core/ObjModelDefs.h>
#include <baselib/core/BaseIncludes.h>
#include <baselib/core/TimeUtils.h>

namespace utest
{
    /*
     * This class mocks AuthorizationCache in order to remove dependency on Janus
     * for some unit tests, which in turn makes them more robust.
     */

    template
    <
        typename E = void
    >
    class DummyAuthorizationCacheT : public bl::security::AuthorizationCache
    {
        BL_CTR_DEFAULT( DummyAuthorizationCacheT, protected )

        BL_DECLARE_OBJECT_IMPL_ONEIFACE( DummyAuthorizationCacheT, AuthorizationCache )

    protected:

        static const std::string                                                    g_dummyTokenType;
        static const std::string                                                    g_dummyTokenData;

    public:

        static auto dummyTokenType() NOEXCEPT -> const std::string&
        {
            return g_dummyTokenType;
        }

        static auto dummyTokenData() NOEXCEPT -> const std::string&
        {
            return g_dummyTokenData;
        }

        static auto getTestSecurityPrincipal(
            SAA_in_opt          const bl::om::ObjPtr< bl::data::DataBlock >&        authenticationToken = nullptr
            ) -> bl::om::ObjPtr< bl::security::SecurityPrincipal >
        {
            return bl::security::SecurityPrincipal::createInstance(
                "sid1234",
                "John",
                "Smith",
                "john.smith@host.com",
                "principalType1",
                bl::om::copy( authenticationToken )
                );
        }

        virtual auto tokenType() const NOEXCEPT -> const std::string& OVERRIDE
        {
            return dummyTokenType();
        }

        virtual void configureFreshnessInterval(
            SAA_in_opt          const bl::time::time_duration&                      freshnessInterval = bl::time::neg_infin
            ) OVERRIDE
        {
            BL_UNUSED( freshnessInterval );

            BL_THROW(
                bl::NotSupportedException(),
                "DummyAuthorizationCache::configureFreshnessInterval() is unimplemented"
                );
        }

        virtual auto tryGetAuthorizedPrinciplal(
            SAA_in              const bl::om::ObjPtr< bl::data::DataBlock >&            authenticationToken
            )
            -> bl::om::ObjPtr< bl::security::SecurityPrincipal > OVERRIDE
        {
            std::string tokenData;

            tokenData.assign( authenticationToken -> begin(), authenticationToken -> end() );

            if( "<throw>" != tokenData )
            {
                return getTestSecurityPrincipal( authenticationToken );
            }

            BL_THROW(
                bl::SecurityException(),
                BL_MSG()
                    << "Authorization request has failed"
                );
        }

        virtual auto createAuthorizationTask(
            SAA_in              const bl::om::ObjPtr< bl::data::DataBlock >&            authenticationToken
            )
            -> bl::om::ObjPtr< bl::tasks::Task > OVERRIDE
        {
            BL_UNUSED( authenticationToken );
            BL_THROW(
                bl::NotSupportedException(),
                "DummyAuthorizationCache::createAuthorizationTask() is unimplemented"
                );
        }

        virtual auto tryUpdate(
            SAA_in              const bl::om::ObjPtr< bl::data::DataBlock >&            authenticationToken,
            SAA_in_opt          const bl::om::ObjPtr< bl::tasks::Task >&                authorizationTask = nullptr
            )
            -> bl::om::ObjPtr< bl::security::SecurityPrincipal > OVERRIDE
        {
            BL_UNUSED( authenticationToken );
            BL_UNUSED( authorizationTask );
            BL_THROW(
                bl::NotSupportedException(),
                "DummyAuthorizationCache::tryUpdate() is unimplemented"
                );
        }

        virtual auto update(
            SAA_in              const bl::om::ObjPtr< bl::data::DataBlock >&            authenticationToken,
            SAA_in_opt          const bl::om::ObjPtr< bl::tasks::Task >&                authorizationTask = nullptr
            )
            -> bl::om::ObjPtr< bl::security::SecurityPrincipal > OVERRIDE
        {
            BL_UNUSED( authenticationToken );
            BL_UNUSED( authorizationTask );
            BL_THROW(
                bl::NotSupportedException(),
                "DummyAuthorizationCache::update() is unimplemented"
                );
        }

        virtual void evict(
            SAA_in              const bl::om::ObjPtr< bl::data::DataBlock >&            authenticationToken
            ) OVERRIDE
        {
            BL_UNUSED( authenticationToken );
            BL_THROW(
                bl::NotSupportedException(),
                "DummyAuthorizationCache::evict() is unimplemented"
                );
        }
    };

    BL_DEFINE_STATIC_CONST_STRING( DummyAuthorizationCacheT, g_dummyTokenType ) = "DummyTokenType";
    BL_DEFINE_STATIC_CONST_STRING( DummyAuthorizationCacheT, g_dummyTokenData ) = "DummyTokenData";

    typedef bl::om::ObjectImpl< DummyAuthorizationCacheT<> > DummyAuthorizationCache;

    /**
     * @brief Helpers for aiding implementation of messaging tests
     */

    template
    <
        typename E = void
    >
    class TestMessagingUtilsT
    {
        BL_DECLARE_STATIC( TestMessagingUtilsT )

    protected:

        static std::string                                                                  g_tokenType;
        static bl::os::mutex                                                                g_tokenTypeLock;

        typedef bl::om::ObjectImpl
        <
            bl::security::AuthorizationCacheImpl< bl::security::AuthorizationServiceRest >
        >
        cache_t;

    public:

        typedef bl::data::datablocks_pool_type                                              datablocks_pool_t;
        typedef bl::tasks::ExecutionQueue                                                   ExecutionQueue;
        typedef bl::messaging::BackendProcessing                                            BackendProcessing;

        typedef bl::messaging::MessagingClientBlockDispatch                                 block_dispatch_t;
        typedef bl::messaging::MessagingClientObjectDispatch                                object_dispatch_t;

        typedef bl::messaging::MessagingClientFactorySsl                                    client_factory_t;
        typedef client_factory_t::messaging_client_t                                        client_t;

        typedef client_factory_t::connection_establisher_t                                  connector_t;
        typedef client_factory_t::async_wrapper_t                                           async_wrapper_t;

        typedef client_factory_t::sender_connection_t                                       sender_connection_t;
        typedef client_factory_t::receiver_connection_t                                     receiver_connection_t;

        typedef bl::tasks::TaskControlTokenRW                                               token_t;
        typedef bl::om::ObjPtrCopyable< token_t >                                           token_ptr_t;
        typedef bl::om::ObjPtrCopyable< datablocks_pool_t >                                 pool_ptr_t;

        typedef bl::messaging::BrokerProtocol                                               BrokerProtocol;
        typedef bl::messaging::Payload                                                      Payload;

        typedef std::vector
            <
                std::pair< bl::om::ObjPtr< connector_t >, bl::om::ObjPtr< connector_t > >
            >
            connections_list_t;

        typedef std::vector
            <
                std::pair< bl::uuid_t, bl::om::ObjPtrDisposable< bl::messaging::MessagingClientObject > >
            >
            clients_list_t;

        typedef bl::cpp::function
            <
                void (
                    SAA_in          const std::string&                                      cookiesText,
                    SAA_in          const bl::om::ObjPtr< datablocks_pool_t >&              dataBlocksPool,
                    SAA_in          const bl::om::ObjPtr< ExecutionQueue >&                 eq,
                    SAA_in          const bl::om::ObjPtr< BackendProcessing >&              backend,
                    SAA_in          const bl::om::ObjPtr< async_wrapper_t >&                asyncWrapper
                    )
            >
            callback_t;

        static auto createConnections( SAA_in const std::size_t maxConcurrentConnections = 16U ) -> connections_list_t
        {
            BL_CHK(
                false,
                test::UtfArgsParser::connections() >= 2U,
                BL_MSG()
                    << "The minimum value of the --connections parameter is 2"
                );

            return createNoOfConnections(
                test::UtfArgsParser::connections()      /* noOfConnections */,
                test::UtfArgsParser::host(),
                test::UtfArgsParser::port(),
                maxConcurrentConnections
                );
        }

        static auto createNoOfConnections(
            SAA_in          const std::size_t                                               noOfConnections,
            SAA_in_opt      const std::string&                                              host = test::UtfArgsParser::host(),
            SAA_in_opt      const unsigned short                                            port = test::UtfArgsParser::port(),
            SAA_in_opt      const std::size_t                                               maxConcurrentConnections = 16U
            )
            -> connections_list_t
        {
            using namespace bl;
            using namespace bl::tasks;
            using namespace bl::messaging;

            BL_LOG(
                bl::Logging::debug(),
                BL_MSG()
                    << "Establishing broker connections for "
                    << noOfConnections
                    << " test messaging clients"
                );

            const auto inboundQueue = lockDisposable(
                ExecutionQueueImpl::createInstance< ExecutionQueue >(
                    ExecutionQueue::OptionKeepAll,
                    maxConcurrentConnections
                    )
                );

            const auto outboundQueue = lockDisposable(
                ExecutionQueueImpl::createInstance< ExecutionQueue >(
                    ExecutionQueue::OptionKeepAll,
                    maxConcurrentConnections
                    )
                );

            for( std::size_t i = 0; i < noOfConnections; ++i )
            {
                inboundQueue -> push_back( connector_t::createInstance< Task >( bl::cpp::copy( host ), port ) );

                outboundQueue -> push_back( connector_t::createInstance< Task >( bl::cpp::copy( host ), port + 1 ) );
            }

            const auto startTime = time::second_clock::universal_time();

            for( ;; )
            {
                os::sleep( time::seconds( 2 ) );

                if(
                    ! inboundQueue -> hasPending()      &&
                    ! inboundQueue -> hasExecuting()    &&
                    ! outboundQueue -> hasPending()     &&
                    ! outboundQueue -> hasExecuting()
                    )
                {
                    break;
                }

                const auto elapsed = time::second_clock::universal_time() - startTime;

                /*
                 * Note: the checks below are to handle an apparently OS issue with connect
                 * operations hanging occasionally
                 */

                if( elapsed > time::minutes( 5 ) )
                {
                    inboundQueue -> cancelAll( false /* wait */ );
                    outboundQueue -> cancelAll( false /* wait */ );
                }

                if( elapsed > time::minutes( 15 ) )
                {
                    BL_RIP_MSG( "Connection tasks are hung more than 15 minutes" );
                }
            }

            UTF_REQUIRE_EQUAL( inboundQueue -> size(), noOfConnections );
            UTF_REQUIRE_EQUAL( inboundQueue -> size(), outboundQueue -> size() );

            connections_list_t result;
            result.reserve( noOfConnections );

            for( std::size_t i = 0; i < noOfConnections; ++i )
            {
                result.push_back(
                    std::make_pair(
                        om::qi< connector_t >( inboundQueue -> pop() )          /* inboundConnection */,
                        om::qi< connector_t >( outboundQueue -> pop() )         /* outboundConnection */
                        )
                    );
            }

            return result;
        }

        static auto getTokenData() -> const std::string&
        {
            return (
                ( test::UtfArgsParser::path().empty() || test::UtfArgsParser::password().empty() ) ?
                    DummyAuthorizationCache::dummyTokenData()
                    :
                    test::UtfArgsParser::password()
                );
        }

        static auto createBrokerProtocolMessage(
            SAA_in                  const bl::messaging::MessageType::Enum          messageType,
            SAA_in                  const bl::uuid_t&                               conversationId,
            SAA_in_opt              const std::string&                              cookiesText,
            SAA_in_opt              const bl::uuid_t&                               messageId = bl::uuids::create()
            )
            -> bl::om::ObjPtr< bl::messaging::BrokerProtocol >
        {
            {
                BL_MUTEX_GUARD( g_tokenTypeLock );

                if( g_tokenType.empty() )
                {
                    using namespace bl::security;

                    g_tokenType =
                        ( test::UtfArgsParser::path().empty() || test::UtfArgsParser::password().empty() ) ?
                            DummyAuthorizationCache::dummyTokenType()
                            :
                            cache_t::template createInstance< AuthorizationCache >(
                                AuthorizationServiceRest::create( test::UtfArgsParser::path() )
                                )
                                -> tokenType();
                }
            }

            return bl::messaging::MessagingUtils::createBrokerProtocolMessage(
                messageType,
                conversationId,
                g_tokenType,
                cookiesText,
                messageId
                );
        }

        static void executeMessagingTests(
            SAA_in          const bl::om::ObjPtr< object_dispatch_t >&                      incomingObjectChannel,
            SAA_in          const callback_t&                                               callback
            )
        {
            using namespace bl;
            using namespace bl::messaging;

            tasks::scheduleAndExecuteInParallel(
                [ & ]( SAA_in const om::ObjPtr< ExecutionQueue >& eq ) -> void
                {
                    eq -> setOptions( ExecutionQueue::OptionKeepNone );

                    const auto& cookiesText = getTokenData();

                    const auto dataBlocksPool = datablocks_pool_t::createInstance();

                    const auto incomingBlockChannel = om::lockDisposable(
                        MessagingClientBlockDispatchFromObject::createInstance< MessagingClientBlockDispatch >(
                            om::copy( incomingObjectChannel )
                            )
                        );

                    const auto backend = om::lockDisposable(
                        client_factory_t::createClientBackendProcessingFromBlockDispatch(
                            om::copy( incomingBlockChannel )
                            )
                        );

                    const auto asyncWrapper = om::lockDisposable(
                        client_factory_t::createAsyncWrapperFromBackend(
                            om::copy( backend ),
                            0U              /* threadsCount */,
                            0U              /* maxConcurrentTasks */,
                            om::copy( dataBlocksPool )
                            )
                        );

                    callback( cookiesText, dataBlocksPool, eq, backend, asyncWrapper );

                    eq -> flush();

                    UTF_REQUIRE( eq -> isEmpty() );
                }
                );
        }

        static void waitForKeyOrTimeout()
        {
            if( test::UtfArgsParser::timeoutInSeconds() == 0 )
            {
                BL_LOG(
                    bl::Logging::debug(),
                    BL_MSG()
                        << "Press [Enter] to stop messaging client tests"
                    );

                ::getchar();
            }
            else
            {
                BL_LOG(
                    bl::Logging::debug(),
                    BL_MSG()
                        << "Executed messaging client tests for "
                        << test::UtfArgsParser::timeoutInSeconds()
                        << " seconds"
                    );

                bl::os::sleep(
                    bl::time::seconds( static_cast< long >( test::UtfArgsParser::timeoutInSeconds() ) )
                    );
            }

            BL_LOG(
                bl::Logging::debug(),
                "Closing all messaging client connections"
                );
        }

        static void dispatchCallback(
            SAA_in              const bl::om::ObjPtrCopyable< bl::om::Proxy >&  clientSink,
            SAA_in_opt          const bl::uuid_t&                               targetPeerIdExpected,
            SAA_in              const bl::uuid_t&                               targetPeerId,
            SAA_in              const bl::om::ObjPtr< BrokerProtocol >&         brokerProtocol,
            SAA_in_opt          const bl::om::ObjPtr< Payload >&                payload
            )
        {
            if( targetPeerIdExpected != bl::uuids::nil() )
            {
                UTF_REQUIRE_EQUAL( targetPeerId, targetPeerIdExpected );
            }

            bl::os::mutex_unique_lock guard;

            {
                const auto target = clientSink -> tryAcquireRef< object_dispatch_t >(
                    object_dispatch_t::iid(),
                    &guard
                    );

                if( target )
                {
                    target -> pushMessage( targetPeerId, brokerProtocol, payload );
                }
            }
        }

        static void verifyUniformMessageDistribution( SAA_in const clients_list_t& clients )
        {
            using namespace bl;

            /*
             * Verify that all channels dispatched at least 2 messages or more
             * including the incoming channels
             */

            std::uint64_t receivedLower = std::numeric_limits< std::uint64_t >::max();
            std::uint64_t receivedUpper = std::numeric_limits< std::uint64_t >::min();

            std::uint64_t sentLower = std::numeric_limits< std::uint64_t >::max();
            std::uint64_t sentUpper = std::numeric_limits< std::uint64_t >::min();

            for( std::size_t i = 0U, count = clients.size(); i < count; ++i )
            {
                const auto& client = clients[ i ].second;

                const auto clientImpl = om::qi< client_t >( client -> outgoingBlockChannel() );

                UTF_REQUIRE( clientImpl -> noOfBlocksReceived() > 2U );
                UTF_REQUIRE( clientImpl -> noOfBlocksSent() > 2U );

                if( clientImpl -> noOfBlocksReceived() < receivedLower )
                {
                    receivedLower = clientImpl -> noOfBlocksReceived();
                }

                if( clientImpl -> noOfBlocksReceived() > receivedUpper )
                {
                    receivedUpper = clientImpl -> noOfBlocksReceived();
                }

                if( clientImpl -> noOfBlocksSent() < sentLower )
                {
                    sentLower = clientImpl -> noOfBlocksSent();
                }

                if( clientImpl -> noOfBlocksSent() > sentUpper )
                {
                    sentUpper = clientImpl -> noOfBlocksSent();
                }
            }

            BL_LOG_MULTILINE(
                Logging::debug(),
                BL_MSG()
                    << "Message bounds: "
                    << "receivedLower="
                    << receivedLower
                    << "; receivedUpper="
                    << receivedUpper
                    << "; sentLower="
                    << sentLower
                    << "; sentUpper="
                    << sentUpper
                );
        }

        static void flushQueueWithRetriesOnTargetPeerNotFound( SAA_in const bl::om::ObjPtr< ExecutionQueue >& eq )
        {
            using namespace bl;
            using namespace bl::tasks;
            using namespace bl::messaging;

            const std::size_t maxRetries = 2000U;
            std::map< Task*, std::size_t > retryCountsMap;

            for( ;; )
            {
                const auto task = eq -> top( true /* wait */ );

                if( ! task )
                {
                    BL_ASSERT( eq -> isEmpty() );

                    break;
                }

                if( ! task -> isFailed() )
                {
                    eq -> pop( true /* wait */ );

                    continue;
                }

                /*
                 * The task has failed - check if we need to do a retry
                 */

                try
                {
                    cpp::safeRethrowException( task -> exception() );
                }
                catch( ServerErrorException& e )
                {
                    const auto* ec = e.errorCode();

                    if( ec && eh::errc::make_error_code( BrokerErrorCodes::TargetPeerNotFound ) == *ec )
                    {
                        auto& retryCount = retryCountsMap[ task.get() ];

                        if( retryCount >= maxRetries )
                        {
                            throw;
                        }

                        ++retryCount;

                        os::sleep( time::milliseconds( 200L ) );

                        eq -> push_back( task );

                        continue;
                    }

                    throw;
                }
            }
        }

        static auto getDefaultProxyInboundPort() -> unsigned short
        {
            return test::UtfArgsParser::port() + 2U;
        }

        static void startBrokerProxy(
            SAA_in_opt          const token_ptr_t&                  controlToken = nullptr,
            SAA_in_opt          const bl::cpp::void_callback_t&     callback = bl::cpp::void_callback_t(),
            SAA_in_opt          const unsigned short                proxyInboundPort = getDefaultProxyInboundPort(),
            SAA_in_opt          const std::size_t                   noOfConnections = test::UtfArgsParser::connections(),
            SAA_in_opt          const std::string&                  brokerHostName = test::UtfArgsParser::host(),
            SAA_in_opt          const unsigned short                brokerInboundPort = test::UtfArgsParser::port(),
            SAA_in_opt          const pool_ptr_t&                   dataBlocksPool = datablocks_pool_t::createInstance(),
            SAA_in_opt          const bl::time::time_duration&      heartbeatInterval = bl::time::neg_infin,
            SAA_inout_opt       bl::om::Object**                    backendRef = nullptr
            )
        {
            using namespace bl;
            using namespace bl::data;
            using namespace bl::messaging;

            const auto controlTokenLocal =
                controlToken ?
                    bl::om::copy( controlToken )
                    :
                    tasks::SimpleTaskControlTokenImpl::createInstance< tasks::TaskControlTokenRW >();

            std::vector< std::string > endpoints;

            cpp::SafeOutputStringStream os;

            os
                << brokerHostName
                << ":"
                << brokerInboundPort;

            const auto endpoint = os.str();

            endpoints.push_back( endpoint );
            endpoints.push_back( endpoint );
            endpoints.push_back( endpoint );

            const auto peerId = uuids::create();

            BL_LOG(
                Logging::debug(),
                BL_MSG()
                    << "Proxy clients peerId: "
                    << peerId
                );

            /*
             * For the unit test case we want to pass waitAllToConnect=true
             *
             * Also for testing purpose we want to set maxNoOfSmallBlocks and minSmallBlocksDeltaToLog
             * to some small values, so we can test correctly all the code paths and the logging, etc
             */

            const auto maxNoOfSmallBlocks = test::UtfArgsParser::isServer() ?
                50 * test::UtfArgsParser::connections() : test::UtfArgsParser::connections();

            const auto minSmallBlocksDeltaToLogDefault = maxNoOfSmallBlocks / 10U;

            const auto proxyBackend = bl::om::lockDisposable(
                ProxyBrokerBackendProcessingFactorySsl::create(
                    test::UtfArgsParser::port()                     /* defaultInboundPort */,
                    bl::om::copy( controlTokenLocal ),
                    peerId,
                    noOfConnections,
                    std::move( endpoints ),
                    dataBlocksPool,
                    0U                                              /* threadsCount */,
                    0U                                              /* maxConcurrentTasks */,
                    true                                            /* waitAllToConnect */,
                    maxNoOfSmallBlocks,
                    minSmallBlocksDeltaToLogDefault < 5U ?
                        5U : minSmallBlocksDeltaToLogDefault        /* minSmallBlocksDeltaToLog */
                    )
                );

            if( backendRef )
            {
                *backendRef = proxyBackend.get();
            }

            bl::messaging::BrokerFacade::execute(
                proxyBackend,
                test::UtfCrypto::getDefaultServerKey()              /* privateKeyPem */,
                test::UtfCrypto::getDefaultServerCertificate()      /* certificatePem */,
                proxyInboundPort                                    /* inboundPort */,
                proxyInboundPort + 1U                               /* outboundPort */,
                test::UtfArgsParser::threadsCount(),
                0U                                                  /* maxConcurrentTasks */,
                callback,
                om::copy( controlTokenLocal ),
                dataBlocksPool,
                cpp::copy( heartbeatInterval )
                );
        }

        static auto createTestMessagingBackend() -> bl::om::ObjPtr< bl::messaging::BackendProcessing >
        {
            using namespace bl;
            using namespace bl::security;

            return messaging::BrokerBackendProcessing::createInstance< messaging::BackendProcessing >(
                ( test::UtfArgsParser::path().empty() || test::UtfArgsParser::password().empty() ) ?
                    DummyAuthorizationCache::createInstance< AuthorizationCache >()
                    :
                    cache_t::template createInstance< AuthorizationCache >(
                        AuthorizationServiceRest::create( test::UtfArgsParser::path() )
                        )
                );
        }
    };

    BL_DEFINE_STATIC_STRING( TestMessagingUtilsT, g_tokenType );
    BL_DEFINE_STATIC_MEMBER( TestMessagingUtilsT, bl::os::mutex, g_tokenTypeLock );

    typedef TestMessagingUtilsT<> TestMessagingUtils;

} // utest

#endif /* __UTEST_TESTMESSAGINGUTILS_H_ */
