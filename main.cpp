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

const mach_header * search_module(int task, char* lib_name) {
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
    unsigned int size1 = infos_count * 24; // sizeof(struct dyld_image_info) * infos->infoArrayCount
    int read_res1 = vm_read(task, (unsigned long) infos->infoArray, size1, &mem, &size1);
    is_success(read_res1);
    struct dyld_image_info* info = (struct dyld_image_info*) mem;
    cout << "info mem " << mem << endl;
    unsigned int lim = 512;
    for (int i = 0; i < infos_count; i++) {
      vm_read(task, (unsigned long) info[i].imageFilePath, lim, &mem, &lim);
      char *lib_path = (char *) mem;
      // cout << "lib_path " << lib_path << endl;
      if (strstr(lib_path, lib_name) != NULL) {
        auto lib = info[i].imageLoadAddress;
        return lib; // todo: convert to proper type
      }
    }
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

  auto client = search_module(task, "/client.dylib");
  cout << "CLIENT " << client << endl;

  return 0;
}
