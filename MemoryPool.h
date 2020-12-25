#pragma once

#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <vector>
#include <mutex>
#include <memory>

namespace mempool {
	using ULL = unsigned long long;
	using VOIDPTR = void*;
	using SHORTINT = short int;
	static const short int MEMPOOL_OK = 0;
	static const short int MEMPOOL_PARAMETER_ERROR = 1;
	static const short int MEMPOOL_QUEUE_EMPYT = 2;
	static const short int MEMPOOL_NULL_PTR = 3;
	static const short int MEMPOOL_INVALID_PTR = 4;

	static std::map<SHORTINT, std::string> errStrMap = {
		{MEMPOOL_OK, "成功"},
		{MEMPOOL_PARAMETER_ERROR, "参数错误"},
	    {MEMPOOL_QUEUE_EMPYT, "队列为空，吴内存可供分配"},
	    {MEMPOOL_NULL_PTR,    "指针为空"},
	    {MEMPOOL_INVALID_PTR, "不是池子里的指针，无效"}
	};

	class MemoryPool
	{
	public:
		MemoryPool() = default;
		~MemoryPool() = default;

		// 初始化内存池
		int initPool(const size_t blocksize, const size_t blocks);
		
		// 从内存池里分配一块内存供使用
		int malloc_block(void** ptr);

		// 是否一块已使用完毕的内存
		int free_block(void* ptr);

		// 根据错误码返回错误描述
		std::string getErrStr(const SHORTINT n);

		// 逆初始内存池
		int unInitPool();

	private:
		// 一共有多少块小内存，每块的大小为block_size;
		size_t  blocks_ = 0;

		// 每小块内存的大小
		size_t  block_size_ = 0;

		// 保存内存块首地址的数值
		std::queue<ULL> block_queue_;

		// 存储可用的内存块首地址的数值和地址
		std::map<ULL, VOIDPTR> block_map_;

		// 保护队列互斥锁
		std::mutex queue_mtx_;

		// 保护map的互斥锁
		std::mutex map_mtx_;

		// 条件变量
		std::condition_variable cond_;

		// 存储内存块的共享指针
		std::vector<std::shared_ptr<void>> ptr_vec_;
	};
}


