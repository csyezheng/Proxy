//
// Created by csyezheng on 11/22/16.
//

#ifndef PROXY_TEMP_H
#define PROXY_TEMP_H

std::thread producer([&]()
                     {
                         for (int i = 0; i < 5; ++i)
                         {
                             std::this_thread::sleep_for(std::chrono::seconds(1));
                             std::unique_lock <std::mutex> lock(m);
                             std::cout << "producing" << i << "\n";
                             tasks.push(i);
                             notified = true;
                             cond_var.notify_one();
                         }
                         done = true;
                         cond_var.notify_one();
                     });
std::thread consumer([&]()
                     {
                         std::unique_lock <std::mutex> lock(m);
                         while (!done)
                         {
                             while (!notified)
                             {
                                 cond_var.wait(lock);
                             }
                             while (!produced_nums.empty())
                             {
                                 std::cout << "codfsa:" << endl;
                                 produced_nums.pop();
                             }
                             notified = false;
                         }
                     });

#endif //PROXY_TEMP_H
