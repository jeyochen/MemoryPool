#include "MemoryPool.h"

#include <chrono>

namespace mempool {
	// 初始化内存池
	int MemoryPool::initPool(const size_t blocksize, const size_t blocks) {
		if (0 >= blocksize || 0 >= blocks) {
			return MEMPOOL_PARAMETER_ERROR;
		}

		block_size_ = blocksize;
		blocks_ = blocks;

		std::lock_guard<std::mutex> map_lock(map_mtx_);
		std::lock_guard<std::mutex> queue_lock(queue_mtx_);
		size_t cnt = 0;
		while(cnt < blocks) {
			void* ptr = (void *)(new char[blocksize]);
			if (ptr != nullptr) {
				//初始化内存
				memset(ptr, 0, blocksize);

				// 把地址转换为整数
				ULL memIdx = reinterpret_cast<ULL>(ptr);

				// 保存可用的内存首地址
				block_map_[memIdx] = ptr;

				// 记录可用的内存块索引
				block_queue_.push(memIdx);

				// 构造共享指针，自动释放内存
				std::shared_ptr<void> sptr(ptr, [](void* p) {
					std::cout << "delete ptr:" << p << std::endl; 
					delete p;
					p = NULL;
				});
				ptr_vec_.push_back(sptr);

				std::cout << "memory index:" << memIdx << ", addr:" << (void *)ptr << std::endl;
			}
			else {
				std::cout << "new blocksize failed." << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			++cnt;
		}

		return MEMPOOL_OK;
	}

	// 从内存池里分配一块内存供使用
	int MemoryPool::malloc_block(void** ptr) {

		while (!block_queue_.empty()) {
			// 检查队列里是否有内存可供分配
			std::unique_lock<std::mutex>  queue_lock(queue_mtx_);
			if (block_queue_.empty()) {
				std::cout << "block queue empty, malloc memory block failed." << std::endl;
				return MEMPOOL_QUEUE_EMPYT;
			}

			ULL idx = block_queue_.front();
			block_queue_.pop();
			queue_lock.unlock();

			// 检查map里对应的内存地址是否有效
			std::unique_lock<std::mutex>  map_lock(map_mtx_);
			if (block_map_.find(idx) != block_map_.end() && block_map_[idx] != NULL) {
				*ptr = block_map_[idx];
				block_map_[idx] = NULL; // 标记指针已经分配出去了
				map_lock.unlock();
				break;
			}
		}

		return MEMPOOL_OK;
	}

	// 释放一块已使用完毕的内存
	int MemoryPool::free_block(void* ptr) {
		if (NULL == ptr) {
			return MEMPOOL_NULL_PTR;
		}

		// 先检查是否是从池子里分配出去的内存
		ULL idx = reinterpret_cast<ULL>(ptr);
		std::unique_lock<std::mutex>  map_lock(map_mtx_);
		if (block_map_.find(idx) == block_map_.end()) {
			return MEMPOOL_INVALID_PTR;
		}
		block_map_[idx] = ptr;
		memset(ptr, 0, block_size_);

		std::unique_lock<std::mutex>  queue_lock(queue_mtx_);
		block_queue_.push(idx);

		return MEMPOOL_OK;
	}

    // 根据错误码返回错误描述
	std::string MemoryPool::getErrStr(const SHORTINT n) {
		std::string estr{ "未知错误" };
		std::map<SHORTINT, std::string>::iterator iter = errStrMap.find(n);
		if (iter != errStrMap.end()) {
			estr = iter->second;
		}

		return estr;
	}

	// 逆初始内存池
	int MemoryPool::unInitPool() {
		std::unique_lock<std::mutex> qlck(queue_mtx_);
		while (!block_queue_.empty()) {
			block_queue_.pop();
		}
		qlck.unlock();

		std::unique_lock<std::mutex> mlck(map_mtx_);
		block_map_.clear();
		mlck.unlock();

		ptr_vec_.clear();

		return MEMPOOL_OK;
	}
}
