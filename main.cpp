#include <boost/json.hpp>
#include <boost/locale.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

struct AppSettings {
  struct SheBao {
    struct Item {
      float geren = 0.0f;
      float qiye = 0.0f;
      std::string name;
    };
    std::vector<Item> items;
  } shebao;

  struct ShuiLv {
    struct Item {
      float min = 0.0f;
      float max = 0.0f;
      float rate = 0.0f;
      float deduction = 0.0f;
      std::string name;
    };
    std::vector<Item> items;
  } shuilv;
};

struct UserData {
  float shuiqian = 0.0f;
  float gjj_jishu = 0.0f;
  float gjj_bili = 0.0f;
  float shebao_jishu = 0.0f;
  float qizhengdian = 0.0f;
  float fujia_kouchu = 0.0f;
};

struct CalcResult {
  float gjj = 0.0f;
  struct SheBao {
    struct Result {
      float geren = 0.0f;
      float qiye = 0.0f;
    };
    std::vector<Result> results;
  } shebao;
  struct Level {
    float min = 0.0f;
    float max = 0.0f;
    float rate = 0.0f;
    float deduction = 0.0f;
    std::string name;
  } level;
  float geren_kouchu = 0.0f;
  float qiye_kouchu = 0.0f;
  float shuiqian = 0.0f;
  float geshui = 0.0f;
  float shuihou = 0.0f;
};

std::string U2A(const std::string& text) {
  return boost::locale::conv::between(text, "gb2312", "utf-8");
}

std::vector<char> ReadFileAll(const std::string& path) {
  std::ifstream ifs;
  ifs.open(path, std::ios::in | std::ios::binary);
  if (!ifs.is_open()) {
    throw std::runtime_error("Cannot open file: " + path);
  }
  ifs.seekg(0, std::ios::end);
  auto len = static_cast<std::size_t>(ifs.tellg());
  ifs.seekg(0, std::ios::beg);
  std::vector<char> buf(len);
  ifs.read(buf.data(), len);
  ifs.close();
  return buf;
}

AppSettings LoadAppSettings() {
  AppSettings settings;
  auto buf = ReadFileAll("appsettings.json");
  auto jv = boost::json::parse({buf.data(), buf.size()});
  for (auto& obj : jv.at((const char*)u8"社保比例").as_object()) {
    AppSettings::SheBao::Item item;
    item.name = obj.key();
    item.geren = obj.value().at((const char*)u8"个人").to_number<float>();
    item.qiye = obj.value().at((const char*)u8"企业").to_number<float>();
    settings.shebao.items.emplace_back(std::move(item));
  }
  for (auto& obj : jv.at((const char*)u8"税率等级").as_object()) {
    AppSettings::ShuiLv::Item item;
    item.name = obj.key();
    item.min = obj.value().at((const char*)u8"最小").to_number<float>();
    item.max = obj.value().at((const char*)u8"最大").to_number<float>();
    item.rate = obj.value().at((const char*)u8"税率").to_number<float>();
    item.deduction =
        obj.value().at((const char*)u8"速算扣除数").to_number<float>();
    settings.shuilv.items.emplace_back(std::move(item));
  }
  return settings;
}

UserData GetUserData() {
  UserData user;
  std::cout << "请输入税前月薪：";
  std::cin >> user.shuiqian;
  std::cout << "请输入公积金基数：";
  std::cin >> user.gjj_jishu;
  std::cout << "请输入公积金比例：";
  std::cin >> user.gjj_bili;
  std::cout << "请输入社保基数：";
  std::cin >> user.shebao_jishu;
  std::cout << "请输入起征点：";
  std::cin >> user.qizhengdian;
  std::cout << "请输入专项附加扣除总数：";
  std::cin >> user.fujia_kouchu;
  return user;
}

CalcResult Calc51(const AppSettings& settings, const UserData& user) {
  CalcResult result;
  result.gjj = user.gjj_jishu * user.gjj_bili / 100;
  for (const auto& item : settings.shebao.items) {
    CalcResult::SheBao::Result ret;
    ret.geren = user.shebao_jishu * item.geren / 100;
    ret.qiye = user.shebao_jishu * item.qiye / 100;
    result.shebao.results.emplace_back(std::move(ret));
  }
  result.geren_kouchu = result.gjj;
  result.qiye_kouchu = result.gjj;
  for (const auto& item : result.shebao.results) {
    result.geren_kouchu += item.geren;
    result.qiye_kouchu += item.qiye;
  }
  result.shuiqian = user.shuiqian - user.qizhengdian - result.geren_kouchu -
                    user.fujia_kouchu;
  for (const auto& item : settings.shuilv.items) {
    if (result.shuiqian > item.min && result.shuiqian <= item.max) {
      result.level.deduction = item.deduction;
      result.level.max = item.max;
      result.level.min = item.min;
      result.level.name = item.name;
      result.level.rate = item.rate;
      break;
    }
  }
  result.geshui =
      result.shuiqian * result.level.rate / 100 - result.level.deduction;
  result.shuihou = user.shuiqian - result.geren_kouchu - result.geshui;
  return result;
}

void PrintResult(const AppSettings& settings, const UserData& user,
                 const CalcResult& result) {
  std::cout << "公积金扣除 = 公积金基数 * 公积金比例 = " << user.gjj_jishu
            << " * " << user.gjj_bili << "% = " << result.gjj << std::endl;
  for (std::size_t i = 0; i < result.shebao.results.size(); ++i) {
    const auto& ret = result.shebao.results[i];
    const auto& item = settings.shebao.items[i];
    std::cout << U2A(item.name)
              << "个人 = 社保基数 * 比例 = " << user.shebao_jishu << " * "
              << item.geren << "% = " << ret.geren << std::endl;
    std::cout << U2A(item.name)
              << "企业 = 社保基数 * 比例 = " << user.shebao_jishu << " * "
              << item.qiye << "% = " << ret.qiye << std::endl;
  }
  std::cout << "五险一金个人扣除：" << result.geren_kouchu << std::endl;
  std::cout << "五险一金企业扣除：" << result.qiye_kouchu << std::endl;
  std::cout << "需缴纳个税工资数 = 税前工资 - 起征点 - 五险一金个人扣除 - "
               "附加专项扣除 = "
            << user.shuiqian << " - " << user.qizhengdian << " - "
            << result.geren_kouchu << " - " << user.fujia_kouchu << " = "
            << result.shuiqian << std::endl;
  std::cout << "个税等级：" << U2A(result.level.name)
            << "，范围=" << result.level.min << "-" << result.level.max
            << "，税率=" << result.level.rate
            << "%，速算扣除数=" << result.level.deduction << std::endl;
  std::cout
      << "需缴纳个税 = 需缴纳个税的部分 * 当前等级税率 - 当前等级速算扣除数 = "
      << result.shuiqian << " * " << result.level.rate << "% - "
      << result.level.deduction << " = " << result.geshui << std::endl;
  std::cout << "税后所得 = 税前工资 - 五险一金个人扣除 - 个税 = "
            << user.shuiqian << " - " << result.geren_kouchu << " - "
            << result.geshui << " = " << result.shuihou << std::endl;
}

int main() {
  std::cout.setf(std::ios::fixed);
  std::cout.precision(2);
  try {
    auto settings = LoadAppSettings();
    while (true) {
      std::cout << "--------------------------------------" << std::endl;
      auto user = GetUserData();
      std::cout << "输入的数据为：税前月薪=" << user.shuiqian
                << "，公积金基数=" << user.gjj_jishu
                << "，公积金比例=" << user.gjj_bili
                << "%，社保基数=" << user.shebao_jishu
                << "，起征点=" << user.qizhengdian
                << "，专项附加扣除总数=" << user.fujia_kouchu << std::endl;
      auto result = Calc51(settings, user);
      PrintResult(settings, user, result);
    }
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  std::this_thread::sleep_for(std::chrono::seconds(3));
}
