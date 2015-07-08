#include <pebble.h>

//------------------------------------------------
// コンフィギュレーション項目
//------------------------------------------------
// アンチエイジングするか（PebbleTimeのみ有効）
#define CONFIG_ANTIALIASED (true)

//------------------------------------------------
// コード
//------------------------------------------------
static Window *s_window;
static char s_text[32] = "";

// root_layer の FPS（1秒間に何回再描画したか）
static int s_fps;

// タイマーコールバック
#define TIMEOUT_MS (1)
static void s_app_timer_callback(void *data) {
    // 1msec ごとに root_layer に再描画を要求する
    layer_mark_dirty(window_get_root_layer(s_window));
    app_timer_register(TIMEOUT_MS, s_app_timer_callback, NULL);
}

// SELECTボタンハンドラ
static void s_select_click_handler(ClickRecognizerRef recognizer, void *context) {
    s_fps = 0;
    
    // タイマー起動
    app_timer_register(TIMEOUT_MS, s_app_timer_callback, NULL);
}

// ボタンハンドラ登録関数
static void s_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, s_select_click_handler);
}

// root_layer の再描画処理
static void s_root_layer_update_proc(struct Layer *layer, GContext *ctx) {
    // アンチエイジング
#if PBL_PLATFORM_BASALT
    graphics_context_set_antialiased(ctx, CONFIG_ANTIALIASED);
#endif

    // 塗色を白に設定
    graphics_context_set_fill_color(ctx, GColorWhite);
    
    // カウンタが 0 なら背景を白で塗りつぶす
    if (s_fps == 0) {
        graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
    }

    // ランダムな位置に円を描画
    graphics_draw_circle(ctx, GPoint(rand()%144, rand()%168), rand()%100);

    // FPS を表示
    graphics_fill_rect(ctx, GRect(0, 0, 50, 20), 0, GCornerNone);
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx,
                       s_text,
                       fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                       GRect(0, 0, 50, 20),
                       GTextOverflowModeWordWrap,
                       GTextAlignmentLeft,
                       NULL);
    s_fps++;
}

// TickTimerServiceハンドラ
// 1 秒間隔で呼び出される
static void s_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    snprintf(s_text, 32, "%d FPS", s_fps);
    s_fps = 0;
}

// s_window のロード時に呼び出される関数
static void s_window_load(Window *window) {
    // root_layer の再描画処理を登録
    layer_set_update_proc(window_get_root_layer(window), s_root_layer_update_proc);
    
    // 1 秒間隔で呼び出すハンドラを登録
    tick_timer_service_subscribe(SECOND_UNIT, s_tick_handler);
}

// s_window のアンロード時に呼び出される関数
static void s_window_unload(Window *window) {
    tick_timer_service_unsubscribe();
}

// 初期化
static void s_init(void) {
    // s_window を作成
    s_window = window_create();
    
    // ステータスバーを隠してフルスクリーンに
#if PBL_PLATFORM_APLITE
    window_set_fullscreen(s_window, true);
#endif
    
    // ボタンハンドラ登録関数を登録
    window_set_click_config_provider(s_window, s_click_config_provider);
    
    // ロード・アンロード時に呼び出される関数を登録
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = s_window_load,
        .unload = s_window_unload,
    });
    
    // ウィンドウ・スタックに登録
    const bool animated = true;
    window_stack_push(s_window, animated);
}

// 終了処理
static void s_deinit(void) {
    window_destroy(s_window);
}

int main(void) {
    s_init();
    app_event_loop();
    s_deinit();
}