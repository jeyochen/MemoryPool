#include <iostream>
#include <random>
#include "MemoryPool.h"

static mempool::MemoryPool g_mem_pool;

void memTest() {
	std::default_random_engine e;
	std::uniform_int_distribution<int> num(10, 20);
	std::uniform_int_distribution<int> chr(0x41, 0x61);
	for (int i = 0; i < 1000; ++i) {
		void* p = NULL;
		int nret = g_mem_pool.malloc_block(&p);
		if (nret != mempool::MEMPOOL_OK) {
			std::cout << "malloc block failed." << std::endl;
			continue;
		}

		// 生成随机填充序列的长度
		int nRandNum = num(e);

		// 随机取大写字母做填充符
		char ch = chr(e);
		memset(p, ch, nRandNum);
		std::cout << std::this_thread::get_id() << (char*)p << std::endl;
		g_mem_pool.free_block(p);
	}
}
int main(int argc, char* argv[]) {

	g_mem_pool.initPool(1024, 1000);
	std::thread th1(memTest);
	std::thread th2(memTest);
	std::thread th3(memTest);
	std::thread th4(memTest);
	std::thread th5(memTest);
	std::thread th6(memTest);

	th1.join();
	th2.join();
	th3.join();
	th4.join();
	th5.join();
	th6.join();

	system("pause");
	g_mem_pool.unInitPool();

	return 0;
}