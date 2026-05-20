#include <Arduino.h>
#include "commands.h"
#define RXD2 18
#define TXD2 17
#define BUTTON_PIN        21

#define POWER_BUTTON      4
#define STOP_BUTTON       5
#define START_BUTTON      7
#define WASH_BUTTON       2
#define exosome_BUTTON    1
#define openclose_BUTTON  6
#define cargo_BUTTON      42

uint8_t currentPage = 0;
bool prevButtonState = HIGH;
bool pageChanged = false;

unsigned long washStartTime = 0;
bool washOnSent = false;
bool washOffSent = false;
bool prevWashBtnState = HIGH;

bool exosomeIconOn = false;
bool cargoIconOn = false;

bool prevExosomeBtn = HIGH;
bool prevCargoBtn = HIGH;
bool prevStartBtn = HIGH;

bool iconsResetAfterPageChange = true;

int exosomeFocusIndex = 0;  // 0x02 페이지 전용
int cargoFocusIndex = 0;    // 0x03 페이지 전용


unsigned long powerPressTime = 0;
unsigned long stopPressTime = 0 ;
bool prevPowerBtnState = digitalRead(POWER_BUTTON);
bool prevStopBtnState  = digitalRead(STOP_BUTTON);
// 아이콘 전체 OFF 함수
void send_page1_iconOffCommands() {
  Serial.println("아이콘 페이지01 전체 OFF + 중앙 초기화");
  Serial2.write(P2_exosome_off_cmd, 8);
  Serial2.write(P2_cargo_off_cmd, 8);
  Serial2.write(P2_wash_off_cmd, 8);
  Serial2.write(P2_standby_off_cmd, 8);
  Serial2.write(P2_tube_off_cmd, 8);
  Serial2.write(P2_starilization_off_cmd, 8);
  Serial2.write(P2_center_ddds_cmd, 8);   // 이가 수정 필요해보임
}

void send_page2_iconComands(){
  Serial.println("아이콘 페이지02 출력 ");
  
  Serial2.write(P3_exosome_small_on_cmd,8);
}
void send_page3_iconComands(){
Serial.println("아이콘 페이지03 출력 ");
 Serial2.write(P3_cargo_low_on_cmd,8);

}

void sendPageChange(uint8_t page) {
  uint8_t page_cmd[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, page};
  Serial2.write(page_cmd, 10);
  Serial.print("페이지 전환: ");
  Serial.println(page, HEX);
  currentPage = page; // 이거 아주 중요!
  iconsResetAfterPageChange = false;
 

  if (page == 0x01) {
    send_page1_iconOffCommands();
    Serial.println("청소 시작: P2_wash_on_cmd 전송");
    Serial2.write(P2_wash_on_cmd, 8);
    washStartTime = millis();
    washOnSent = true;
    washOffSent = false;
    prevWashBtnState = digitalRead(WASH_BUTTON);
  } else {
    washOnSent = false;
    washOffSent = false;
  }

  if (page == 0x02){
    send_page2_iconComands();
  }

  if (page == 0x03){
    send_page3_iconComands();
      if (cargoIconOn) {
    Serial2.write(P2_cargo_on_cmd, 8);  // 상태 유지용 재전송
  }
  }

  if (page == 0x06){

  }



}


void goadminpage() {
  static bool comboStarted = false;
  static unsigned long comboStartTime = 0;

  bool currPower = digitalRead(POWER_BUTTON);
  bool currStop  = digitalRead(STOP_BUTTON);

  bool bothPressed  = (currPower == LOW && currStop == LOW);
  bool bothReleased = (currPower == HIGH && currStop == HIGH);

  if (bothPressed && !comboStarted) {
    comboStartTime = millis();
    comboStarted = true;
    Serial.println("[✓] 두 버튼 눌림 → 타이머 시작");
  }

  if (comboStarted && bothPressed) {
    unsigned long heldTime = millis() - comboStartTime;
    Serial.print("[✓] 버튼 계속 누름 중... 경과: ");
    Serial.print(heldTime);
    Serial.println(" ms");

    if (heldTime >= 2000 && currentPage != 0x06) {
      Serial.println(" 관리자 모드 진입 (페이지 0x06)");
      sendPageChange(0x06);
      delay(500);
      comboStarted = false;  // 타이머 완료 후 리셋
    }
  }

  if (comboStarted && bothReleased) {
    Serial.println(" 버튼을 뗐음 → 실패 또는 이미 전환됨");
    comboStarted = false;  // 버튼 뗐으면 리셋
  }
}




// 엑소좀 포커스 이동
// void move_exosome_focusleft() {
//   if (exosomeFocusIndex > 0) exosomeFocusIndex--;
//   update_exosome_display();
// }

// void move_exosome_focusright() {
//   if (exosomeFocusIndex < 1) exosomeFocusIndex++;
//   update_exosome_display();
// }

// 엑소좀 포커스 이동
void move_exosome_focusright() {
  if (exosomeFocusIndex > 0) exosomeFocusIndex--;
  update_exosome_display();
}

void move_exosome_focusleft() {
  if (exosomeFocusIndex < 1) exosomeFocusIndex++;
  update_exosome_display();
}



void update_exosome_display() {
  Serial.print("엑소좀 포커스 인덱스: ");
  Serial.println(exosomeFocusIndex);
  
  if (exosomeFocusIndex == 0) {
    Serial2.write(P3_exosome_large_on_cmd, 8);
  } else {
    Serial2.write(P3_exosome_small_on_cmd, 8);
  }
}

// 카고 포커스 이동
void moveCargoFocusLeft() {
  if (cargoFocusIndex > 0) cargoFocusIndex--;
  updateCargoDisplay();
}

void moveCargoFocusRight() {
  if (cargoFocusIndex < 2) cargoFocusIndex++;
  updateCargoDisplay();
}

void updateCargoDisplay() {
  Serial.print("카고 포커스 인덱스: ");
  Serial.println(cargoFocusIndex);

  if (cargoFocusIndex == 0)
    Serial2.write(P3_cargo_low_on_cmd, 8);
  else if (cargoFocusIndex == 1)
    Serial2.write(P3_cargo_medium_on_cmd, 8);
  else if (cargoFocusIndex == 2)
    Serial2.write(P3_cargo_high_on_cmd, 8);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(POWER_BUTTON, INPUT_PULLUP);
  pinMode(STOP_BUTTON, INPUT_PULLUP);
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(WASH_BUTTON, INPUT_PULLUP);
  pinMode(exosome_BUTTON, INPUT_PULLUP);
  pinMode(openclose_BUTTON, INPUT_PULLUP);
  pinMode(cargo_BUTTON, INPUT_PULLUP);

  delay(1000);
  sendPageChange(currentPage);
  
  iconsResetAfterPageChange = false;

  Serial.println("=== 초기 버튼 상태 점검 ===");
  Serial.print("POWER_BUTTON: ");
  Serial.println(digitalRead(POWER_BUTTON));  // 기대값: 1
  Serial.print("STOP_BUTTON : ");
  Serial.println(digitalRead(STOP_BUTTON));   // 기대값: 1
  Serial.println("==========================");
}

void loop() {
  // 아이콘 초기화
  goadminpage();
  if (!iconsResetAfterPageChange) {
    Serial.println("아이콘 상태 초기화 실행");
    exosomeIconOn = false;
    cargoIconOn = false;
    exosomeFocusIndex = 0;
    cargoFocusIndex = 0;
    iconsResetAfterPageChange = true;
  }

  // 페이지 넘기기 버튼 처리
  bool buttonState = digitalRead(BUTTON_PIN);
  if (prevButtonState == HIGH && buttonState == LOW) pageChanged = false;

  if (prevButtonState == LOW && buttonState == HIGH && !pageChanged) {
    currentPage = (currentPage + 1) % 7;
    sendPageChange(currentPage);
    pageChanged = true;
    iconsResetAfterPageChange = false;
    delay(200);
  }
  prevButtonState = buttonState;

  // WASH 타이머 + 버튼 처리
  if (currentPage == 0x01 && washOnSent && !washOffSent) {
    if (millis() - washStartTime >= 15000) {
      Serial.println("15초 경과: 청소 종료");
      Serial2.write(P2_wash_off_cmd, 8);
      Serial2.write(P2_standby_on_cmd, 8);
      washOffSent = true;
    }

    bool currentWashBtnState = digitalRead(WASH_BUTTON);
    if (prevWashBtnState == HIGH && currentWashBtnState == LOW) {
      Serial.println("사용자 WASH 버튼 누름: 청소 종료");
      Serial2.write(P2_wash_off_cmd, 8);
      Serial2.write(P2_standby_on_cmd, 8);
      washOffSent = true;
      delay(200);
    }
    prevWashBtnState = currentWashBtnState;
  }
 

  // 버튼 상태 읽기
  bool currExosomeBtn = digitalRead(exosome_BUTTON);
  bool currCargoBtn   = digitalRead(cargo_BUTTON);

  // EXOSOME 버튼 처리 (← 방향)
  if (prevExosomeBtn == HIGH && currExosomeBtn == LOW) {
    if (currentPage == 0x01) {
      Serial.println("엑소좀 버튼 눌림 (페이지 01)");
      if (!exosomeIconOn) {
        Serial2.write(P2_standby_off_cmd, 8);
        Serial2.write(P2_exosome_on_cmd, 8);
        Serial2.write(P2_cargo_off_cmd, 8);
        exosomeIconOn = true;
        cargoIconOn = false;
        Serial2.write(P2_center_exosome_cmd, 8);
      }
    } else if (currentPage == 0x02) {
      move_exosome_focusleft();  // 엑소좀 ←
    } else if (currentPage == 0x03) {
      moveCargoFocusLeft();      // 카고 ←
    }
  }
  prevExosomeBtn = currExosomeBtn;

  // CARGO 버튼 처리 (→ 방향)
  if (prevCargoBtn == HIGH && currCargoBtn == LOW) {
    if (currentPage == 0x01) {
      Serial.println("카고 버튼 눌림 (페이지 01)");
      if (!cargoIconOn) {
        Serial2.write(P2_standby_off_cmd, 8);
        Serial2.write(P2_cargo_on_cmd, 8);
        Serial2.write(P2_exosome_off_cmd, 8);
        cargoIconOn = true;
        exosomeIconOn = false;
        Serial2.write(P2_center_cargo_cmd, 8);
      }
    } else if (currentPage == 0x02) {
      move_exosome_focusright();  // 엑소좀 →
    } else if (currentPage == 0x03) {
      moveCargoFocusRight();      // 카고 →
    }
  }
  prevCargoBtn = currCargoBtn;

  // START 버튼 처리
  bool currStartBtn = digitalRead(START_BUTTON);
  if (prevStartBtn == HIGH && currStartBtn == LOW) {
    iconsResetAfterPageChange = true;
    if (exosomeIconOn) {
      Serial.println("START → 엑소좀 모드 시작: 페이지 0x02 전환");
      sendPageChange(0x02);
    } else if (cargoIconOn) {
      Serial.println("START → 카고 모드 시작: 페이지 0x03 전환");
      sendPageChange(0x03);
    }
    delay(200);
  }
  prevStartBtn = currStartBtn;

   
}

