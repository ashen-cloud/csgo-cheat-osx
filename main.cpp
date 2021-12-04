#include <iostream>
#include <mach/mach.h>
#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>
#include <mach-o/dyld_images.h>
#include <libproc.h>

using namespace std;

int exec(const char* cmd) {
    char buffer[128];
    string result = "";
    FILE* pipe = popen(cmd, "r");
    while (fgets(buffer, sizeof buffer, pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return stoi(result);
}

bool is_success(int res) {
  if (res == KERN_SUCCESS) {
    cout << "success" << endl;
    return true;
  } else {
    cout << "fuck " << res << endl;
    return false;
  }
}

void search_module(int task) {
  struct task_dyld_info dyld_info;
  unsigned int noop = TASK_DYLD_INFO_COUNT;
  int info = task_info(task, TASK_DYLD_INFO, (int *)&dyld_info, &noop);
  unsigned long addr; // long long?
  if (is_success(info)) {
    addr = dyld_info.all_image_info_addr;
  }
  cout << "module addr " << addr << endl;

  unsigned int size = sizeof(dyld_all_image_infos);
  unsigned long mem;
  int read_res = vm_read(task, addr, size, &mem, &size);
  cout << "memory read content " << mem << endl;

  if (is_success(read_res)) {
    auto infos = (dyld_all_image_infos *) mem;
    int infos_count = infos->infoArrayCount;
    cout << "infos count " << infos_count << endl;
  }
}

int create_task(int pid) {
  unsigned int task = 0;
  auto res = task_for_pid(mach_task_self(), pid, &task);
  if (is_success(res)) {
    return task;
  }
  return -1;
}

int main() {
  int cs_pid = exec("pgrep csgo_osx64");
  cout << "csgo pid " << cs_pid << endl;

  auto task = create_task(cs_pid);

  search_module(task);

  return 0;
}
