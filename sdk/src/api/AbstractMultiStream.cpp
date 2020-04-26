/*
 * AbstractMultiStream.cpp
 *
 * Copyright (C) 2020 by RStudio, PBC
 *
 * Unless you have received this program directly from RStudio pursuant to the terms of a commercial license agreement
 * with RStudio, then this program is licensed to you under the following terms:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "AbstractMultiStream.hpp"

#include <atomic>
#include <api/ResponseTypes.hpp>

#include "JobStatusStream.hpp"

namespace rstudio {
namespace launcher_plugins {
namespace api {

typedef std::map<uint64_t, uint64_t> RequestSequenceMap;

template <typename R, typename ... Args>
struct AbstractMultiStream<R, Args...>::Impl
{
   /**
    * @brief Constructor.
    *
    * @param in_launcherCommunicator    The launcher communicator which will send the responses.
    */
   explicit Impl(comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator) :
      LauncherCommunicator(std::move(in_launcherCommunicator))
   {
   }

   /**
    * @brief Adds a new request ID to the sequences map.
    *
    * @param in_requestId   The new request ID.
    */
   void addRequest(uint64_t in_requestId)
   {
      auto itr = Sequences.find(in_requestId);
      if (itr == Sequences.end())
         Sequences.emplace(in_requestId, 1);
   }

   /**
    * @brief Gets all of the registered requests and their current sequence value.
    *
    * @return All registered request sequences.
    */
   StreamSequences getSequences()
   {
      StreamSequences sequences;
      for (auto& sequence : Sequences)
         sequences.emplace_back(sequence.first, sequence.second++);

      return sequences;
   }

   /**
    * @brief Gets the specified registered requests and their current sequence value.
    *
    * @param in_requestIds      The IDs of the desired request sequences.
    *
    * @return The specified registered request sequences.
    */
   StreamSequences getSequences(std::set<uint64_t> in_requestIds)
   {
      StreamSequences sequences;
      for (auto& sequence : Sequences)
      {
         if (in_requestIds.count(sequence.first) > 0)
            sequences.emplace_back(sequence.first, sequence.second++);
      }

      return sequences;
   }

   /**
    * @brief Unregisters the specified request.
    *
    * @param in_requestId   The ID of the request to unregister.
    */
   void removeRequest(uint64_t in_requestId)
   {
         auto itr = Sequences.find(in_requestId);
         if (itr != Sequences.end())
            Sequences.erase(itr);
   }

   /** The launcher communicator for sending responses. */
   comms::AbstractLauncherCommunicatorPtr LauncherCommunicator;

   /** Keeps track of the next sequence ID for each request. */
   RequestSequenceMap Sequences;
};

// Deleter has to be declared manually because of the template args.
template <typename R, typename ... Args>
void AbstractMultiStream<R, Args...>::ImplDeleter::operator()(AbstractMultiStream::Impl* io_toDelete)
{
   delete io_toDelete;
}

template <typename R, typename ... Args>
AbstractMultiStream<R, Args...>::AbstractMultiStream(
   comms::AbstractLauncherCommunicatorPtr in_launcherCommunicator) :
      m_baseImpl(new Impl(std::move(in_launcherCommunicator)))
{
}

template <typename R, typename ... Args>
bool AbstractMultiStream<R, Args...>::isEmpty() const
{
   bool empty = true;
   LOCK_MUTEX(m_mutex)
   {
      empty = m_baseImpl->Sequences.empty();
   }
   END_LOCK_MUTEX

   return empty;
}

template <typename R, typename ... Args>
void AbstractMultiStream<R, Args...>::removeRequest(uint64_t in_requestId)
{
   LOCK_MUTEX(m_mutex)
   {
      onRemoveRequest(in_requestId);
   }
   END_LOCK_MUTEX
}

template <typename R, typename ... Args>
void AbstractMultiStream<R, Args...>::onAddRequest(uint64_t in_requestId)
{
   m_baseImpl->addRequest(in_requestId);
}

template <typename R, typename ... Args>
void AbstractMultiStream<R, Args...>::sendResponse(Args... in_responseArgs)
{
   StreamSequences sequences = m_baseImpl->getSequences();
   if (!sequences.empty())
   m_baseImpl->LauncherCommunicator->sendResponse(R(sequences, in_responseArgs...));
}

template <typename R, typename ... Args>
void AbstractMultiStream<R, Args...>::sendResponse(const std::set<uint64_t>& in_requestIds, Args... in_responseArgs)
{
   StreamSequences sequences = m_baseImpl->getSequences(in_requestIds);
   if (!sequences.empty())
      m_baseImpl->LauncherCommunicator->sendResponse(R(sequences, in_responseArgs...));
}

template <typename R, typename ... Args>
void AbstractMultiStream<R, Args...>::onRemoveRequest(uint64_t in_requestId)
{
   m_baseImpl->removeRequest(in_requestId);
}

// Template Instantiations =============================================================================================
#define INSTANTIATE_CLASS(R, ...)                                                                  \
template                                                                                           \
void AbstractMultiStream<R, __VA_ARGS__>::ImplDeleter::operator()(AbstractMultiStream::Impl*);     \
template                                                                                           \
AbstractMultiStream<R, __VA_ARGS__>::AbstractMultiStream(comms::AbstractLauncherCommunicatorPtr);  \
template                                                                                           \
bool AbstractMultiStream<R, __VA_ARGS__>::isEmpty() const;                                         \
template                                                                                           \
void AbstractMultiStream<R, __VA_ARGS__>::removeRequest(uint64_t);                                 \
template                                                                                           \
void AbstractMultiStream<R, __VA_ARGS__>::onAddRequest(uint64_t);                                  \
template                                                                                           \
void AbstractMultiStream<R, __VA_ARGS__>::sendResponse(__VA_ARGS__);                               \
template                                                                                           \
void AbstractMultiStream<R, __VA_ARGS__>::sendResponse(const std::set<uint64_t>&, __VA_ARGS__);    \

INSTANTIATE_CLASS(JobStatusResponse, api::JobPtr)


} // namespace api
} // namespace launcher_plugins
} // namespace rstudio
