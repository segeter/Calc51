#include <boost/json.hpp>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

// https://www.shebaotong.com/shebaotong/calculator

/*
* ��������˰����˰�ʱ�
1����˰�����3000Ԫ��˰��3%��
2����˰���3000Ԫ��12000Ԫ��˰��10%��
3����˰���12000Ԫ��25000Ԫ��˰��20%��
4����˰���25000Ԫ��35000Ԫ��˰��25%��
5����˰���35000Ԫ��55000Ԫ��˰��30%��
6����˰���55000Ԫ��80000Ԫ��˰��35%��
7����˰���80000Ԫ��˰��45%��
*/

struct AppSettings {
  struct SheBaoConfig {
    struct Item {
      float geren = 0.0f;
      float qiye = 0.0f;
    };
    Item yanglao;
    Item shiye;
    Item gongshang;
    Item shengyu;
    Item yiliao;
  };

  SheBaoConfig shebao;
};

struct UserSettings {
  float shuiqian = 0.0f;
  float gongjijin_jishu = 0.0f;
  float gongjijin_bili = 0.0f;
  float shebao_jishu = 0.0f;
  float qizhengdian = 0.0f;
  float fujia_kouchu = 0.0f;
};

struct GeShuiJieTi {
  float min = 0.0f;
  float max = 0.0f;
  float rate = 0.0f;
};

GeShuiJieTi kJieTi[] = {{0.0f, 3000.0f, 0.03f},
                        {3000.0f, 12000.0f, 0.1f},
                        {12000.0f, 25000.0f, 0.2f},
                        {25000.0f, 35000.0f, 0.25f},
                        {35000.0f, 55000.0f, 0.3f},
                        {55000.0f, 80000.0f, 0.35f},
                        {80000.0f, std::numeric_limits<float>::max(), 0.45f}};

AppSettings app_settings;
UserSettings user_settings;

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

void LoadAppSettings() {
  auto buf = ReadFileAll("appsettings.json");
  auto jv = boost::json::parse({buf.data(), buf.size()});
  auto& shebao = jv.at("shebao").as_object();
  app_settings.shebao.yanglao.geren =
      shebao.at("yanglao").at("geren").to_number<float>();
  app_settings.shebao.yanglao.qiye =
      shebao.at("yanglao").at("qiye").to_number<float>();
  app_settings.shebao.shiye.geren =
      shebao.at("shiye").at("geren").to_number<float>();
  app_settings.shebao.shiye.qiye =
      shebao.at("shiye").at("qiye").to_number<float>();
  app_settings.shebao.gongshang.geren =
      shebao.at("gongshang").at("geren").to_number<float>();
  app_settings.shebao.gongshang.qiye =
      shebao.at("gongshang").at("qiye").to_number<float>();
  app_settings.shebao.shengyu.geren =
      shebao.at("shengyu").at("geren").to_number<float>();
  app_settings.shebao.shengyu.qiye =
      shebao.at("shengyu").at("qiye").to_number<float>();
  app_settings.shebao.yiliao.geren =
      shebao.at("yiliao").at("geren").to_number<float>();
  app_settings.shebao.yiliao.qiye =
      shebao.at("yiliao").at("qiye").to_number<float>();
}

void Calc51() {
  std::cout << "������˰ǰ���ʣ�";
  std::cin >> user_settings.shuiqian;
  std::cout << "�����빫���������";
  std::cin >> user_settings.gongjijin_jishu;
  std::cout << "�����빫���������";
  std::cin >> user_settings.gongjijin_bili;
  std::cout << "�������籣������";
  std::cin >> user_settings.shebao_jishu;
  std::cout << "�����������㣺";
  std::cin >> user_settings.qizhengdian;
  std::cout << "������ר��ӿ۳�������";
  std::cin >> user_settings.fujia_kouchu;

  std::cout << "˰ǰ���ʣ�" << user_settings.shuiqian << "�������������"
            << user_settings.gongjijin_jishu << "�� �����������"
            << user_settings.gongjijin_bili << "���籣������"
            << user_settings.shebao_jishu << "�������㣺"
            << user_settings.qizhengdian << "��ר��ӿ۳�������"
            << user_settings.fujia_kouchu << std::endl;

  float gongjijin_danbian =
      user_settings.gongjijin_jishu * user_settings.gongjijin_bili / 100;
  printf("�����𵥱� = ��������� * ��������� = %f * %f%% = %f\n",
         user_settings.gongjijin_jishu, user_settings.gongjijin_bili,
         gongjijin_danbian);

  float yanglao_geren =
      user_settings.shebao_jishu * app_settings.shebao.yanglao.geren / 100;
  float yanglao_qiye =
      user_settings.shebao_jishu * app_settings.shebao.yanglao.qiye / 100;
  printf("���ϸ��� = �籣���� * ���� = %f * %f%% = %f\n",
         user_settings.shebao_jishu, app_settings.shebao.yanglao.geren,
         yanglao_geren);
  printf("������ҵ = �籣���� * ���� = %f * %f%% = %f\n",
         user_settings.shebao_jishu, app_settings.shebao.yanglao.qiye,
         yanglao_qiye);

  float shiye_geren =
      user_settings.shebao_jishu * app_settings.shebao.shiye.geren / 100;
  float shiye_qiye =
      user_settings.shebao_jishu * app_settings.shebao.shiye.qiye / 100;
  printf("ʧҵ���� = �籣���� * ���� = %f * %f%% = %f\n",
         user_settings.shebao_jishu, app_settings.shebao.shiye.geren,
         shiye_geren);
  printf("ʧҵ��ҵ = �籣���� * ���� = %f * %f%% = %f\n",
         user_settings.shebao_jishu, app_settings.shebao.shiye.qiye,
         shiye_qiye);

  float gongshang_geren =
      user_settings.shebao_jishu * app_settings.shebao.gongshang.geren / 100;
  float gongshang_qiye =
      user_settings.shebao_jishu * app_settings.shebao.gongshang.qiye / 100;
  printf("���̸��� = �籣���� * ���� = %f * %f%% = %f\n",
         user_settings.shebao_jishu, app_settings.shebao.gongshang.geren,
         gongshang_geren);
  printf("������ҵ = �籣���� * ���� = %f * %f%% = %f\n",
         user_settings.shebao_jishu, app_settings.shebao.gongshang.qiye,
         gongshang_qiye);

  float shengyu_geren =
      user_settings.shebao_jishu * app_settings.shebao.shengyu.geren / 100;
  float shengyu_qiye =
      user_settings.shebao_jishu * app_settings.shebao.shengyu.qiye / 100;
  printf("�������� = �籣���� * ���� = %f * %f%% = %f\n",
         user_settings.shebao_jishu, app_settings.shebao.shengyu.geren,
         shengyu_geren);
  printf("������ҵ = �籣���� * ���� = %f * %f%% = %f\n",
         user_settings.shebao_jishu, app_settings.shebao.shengyu.qiye,
         shengyu_qiye);

  float yiliao_geren =
      user_settings.shebao_jishu * app_settings.shebao.yiliao.geren / 100;
  float yiliao_qiye =
      user_settings.shebao_jishu * app_settings.shebao.yiliao.qiye / 100;
  printf("ҽ�Ƹ��� = �籣���� * ���� = %f * %f%% = %f\n",
         user_settings.shebao_jishu, app_settings.shebao.yiliao.geren,
         yiliao_geren);
  printf("ҽ����ҵ = �籣���� * ���� = %f * %f%% = %f\n",
         user_settings.shebao_jishu, app_settings.shebao.yiliao.qiye,
         yiliao_qiye);

  float kouchu_geren = gongjijin_danbian + yanglao_geren + shiye_geren +
                       gongshang_geren + shengyu_geren + yiliao_geren;
  printf("����һ������ܿ۳���%f\n", kouchu_geren);
  printf("����һ����ҵ�ܽ��ɣ�%f\n", gongjijin_danbian + yanglao_qiye +
                                         shiye_qiye + gongshang_qiye +
                                         shengyu_qiye + yiliao_qiye);

  float shiji_shuiqian = user_settings.shuiqian - user_settings.qizhengdian -
                         kouchu_geren - user_settings.fujia_kouchu;

  printf(
      "����ɸ�˰�Ĳ��� = ˰ǰ���� - ������ - ����һ����˿۳� - ����ר��۳� "
      "= %f - %f - %f - %f = %f\n",
      user_settings.shuiqian, user_settings.qizhengdian, kouchu_geren,
      user_settings.fujia_kouchu, shiji_shuiqian);

  std::int32_t index = 0;
  for (const auto& item : kJieTi) {
    if (shiji_shuiqian >= item.min && shiji_shuiqian < item.max) {
      break;
    }
    ++index;
  }

  printf("��˰���ݵȼ���%d\n", index + 1);

  float geshui = 0.0f;
  float shuiqian = shiji_shuiqian;
  for (std::int32_t i = index; i >= 0; --i) {
    geshui += ((shuiqian - kJieTi[i].min) * kJieTi[i].rate);
    shuiqian = kJieTi[i].min;
  }

  printf("��ɸ�˰��%f\n", geshui);
  printf("˰������ = ˰ǰ���� - ����һ����˿۳� - ��˰ = %f - %f - %f = %f\n",
         user_settings.shuiqian, kouchu_geren, geshui,
         user_settings.shuiqian - kouchu_geren - geshui);
}

int main() {
  try {
    LoadAppSettings();
    while (true) {
      std::cout << "--------------------------------------" << std::endl;
      Calc51();
    }
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  std::this_thread::sleep_for(std::chrono::seconds(3));
}
