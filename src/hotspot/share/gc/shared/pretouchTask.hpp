/*
 * Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_GC_SHARED_PRETOUCH_HPP
#define SHARE_GC_SHARED_PRETOUCH_HPP

#include "gc/shared/workgroup.hpp"

class PretouchTask : public AbstractGangTask {
  char* volatile _cur_addr;
  char* const _start_addr;
  char* volatile _end_addr;
  size_t _page_size;
  size_t _chunk_size;
  volatile uint _n_threads;  // Number of threads participating in pretouch.

public:
  enum TaskStatus{ NotReady, Ready, Done};
private:
  // This is used to prevent threads accessing other members while object
  // is getting updated from expander thread. Then other threads wants to
  // join pretouch can check this flag and return.
  volatile char buf[8];
  volatile size_t _task_status;

public:
  PretouchTask(const char* task_name, char* start_address, char* end_address, size_t page_size, size_t chunk_size, size_t n_threads = 0, size_t task_status = Ready);

  void reinitialize(char* start_address, char *end_addr);
  //void set_end_addr(char* end_addr)       { Atomic::release_store(&_end_addr, end_addr); }
  //void set_cur_addr(char* cur_addr)       { Atomic::release_store(&_cur_addr, end_addr); }

  void set_task_status(TaskStatus status) { Atomic::release_store(&_task_status, (size_t)status);  }
  void set_task_done()                    { Atomic::release_store(&_task_status, (size_t)Done);    }
  void set_task_ready()                   { set_task_status(Ready);    }
  void set_task_notready()                { set_task_status(NotReady); }
  bool is_task_ready()                    { return Atomic::load(&_task_status) == Ready; }
  bool is_task_done()                     { return Atomic::load(&_task_status) == Done;  }

  virtual void work(uint worker_id);

/*
  void print(size_t thread_id) {
    char *sa = Atomic::load(&_start_addr);
    char *ca = Atomic::load(&_cur_addr);
    char *ea = Atomic::load(&_end_addr);
    log_trace(gc,heap)( "   Spinner: thread_id=%ld, _start_addr=%p, _cur_addr=%p, _end_addr=%p, _page_size=%ld, _chunk_size=%ld ", thread_id, sa, ca, ea, _page_size, _chunk_size);
  }
*/

  static void* operator new(size_t size) throw() {
    return CHeapObj<mtGC>::operator new(size);
  }

  static void setup_chunk_size_and_page_size(size_t& chunk_size, size_t& page_size);

  static size_t chunk_size();

  static void pretouch(const char* task_name, char* start_address, char* end_address,
                       size_t page_size, WorkGang* pretouch_gang);

};

#endif // SHARE_GC_SHARED_PRETOUCH_HPP
