#include "storage/b_plus_tree.h"
#include "common/container.h"
#include <string>

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  freopen("input.txt", "r", stdin);
  freopen("output.txt", "w", stdout);
  int n;
  std::cin >> n;
  auto disk_manager = std::make_unique<sjtu::DiskManager>("zcychar_index");
  auto *bpm = new sjtu::BufferPoolManager(1000, disk_manager.get());
  sjtu::page_id_t page_id = bpm->NewPage();
  sjtu::Comparator comparator;
  sjtu::DegradedComparator degraded_comparator;
  sjtu::BPlusTree<sjtu::Key, int, sjtu::Comparator, sjtu::DegradedComparator> tree(
    "test", page_id, bpm, comparator, degraded_comparator);
  std::string opt, index;
  int value;
  for (int i = 1; i <= n; ++i) {
    std::cin >> opt;

    switch (opt[0]) {
      case 'i': {
        std::cin >> index >> value;
        tree.Insert(sjtu::Key(index, value), value);
        break;
      }
      case 'd': {
        std::cin >> index >> value;
        tree.Remove(sjtu::Key(index, value));
        break;
      }
      case 'f': {
        std::cin >> index;
        std::vector<int> result;
        tree.GetAllValue(sjtu::Key(index), &result);
        if (result.empty()) {
          std::cout << "null\n";
        } else {
          for (auto val: result) {
            std::cout << val << ' ';
          }
          std::cout << '\n';
        }
        break;
      }
      default: {
        break;
      }
    }
  }
  bpm->WritePage(page_id).AsMut<sjtu::BPlusTreeHeaderPage>()->next_page_id_ = bpm->GetNextPageId();
  bpm->FlushAllPages();

  // delete bpm;
  return 0;
}
