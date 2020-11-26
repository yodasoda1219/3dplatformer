#include "pch.h"
#include "game.h"
#include "scene.h"
#include "input_manager.h"
#include "debug_tools.h"
#include "graphics/framebuffer.h"
#include "graphics/quad.h"
#include "graphics/opengl/opengl_framebuffer.h"
#include "graphics/opengl/opengl_quad.h"
#include "sound/sound.h"
#include "ui/menu_manager.h"
#include "lua_interpreter.h"
#include "shader_registry.h"
#include "script_registry.h"
#include "util.h"
#include "internal_util.h"
#include "script_module.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace thebanana {
	game* g_game = NULL;
	game::game(const std::string& title) {
		srand(CURRENT_TIME(unsigned int));
		this->m_frame = 0;
		constexpr int width = 1600;
		constexpr int height = 900;
		this->m_window = CreateWindowA(BANANA_WINDOW_CLASS_NAME, title.c_str(), WS_VISIBLE | WS_SYSMENU | WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, HINST_THISCOMPONENT, this);
		RECT r;
		GetWindowRect(this->m_window, &r);
		this->m_aspect_ratio = static_cast<float>(r.right - r.left) / static_cast<float>(r.bottom - r.top);
		this->m_viewport = new opengl_viewport(opengl_viewport::viewport_attribs{ this->m_window, 0, 0, width, height, 4, 6, opengl_viewport::viewport_attribs::passed_window });
		this->m_interpreter = new lua_interpreter;
		this->m_scene = new scene(this);
		this->m_input_manager = new input_manager(this);
		this->m_shader_registry = new shader_registry;
		this->m_script_registry = new script_registry(this);
		this->m_model_registry = new model_registry;
		auto ed = this->m_input_manager->get_enumerated_devices();
		for (size_t i = 0; i < ed.size(); i++) {
			this->m_input_manager->add_device(i);
		}
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		sound::sound_manager::init_decoders();
		sound::set_default_sound_api(sound::sound_api::openal);
		graphics::set_default_graphics_api(graphics::graphics_api::opengl);
		this->m_menu_manager = new ui::menu_manager(this);
		this->m_menu_quad = graphics::quad::create(2.f, 2.f, this->m_menu_manager->get_texture(), true);
		this->m_sound_manager = new sound::sound_manager(this);
		this->m_show_cursor = false;
		this->m_clip_cursor = true;
		this->m_debug_menus_initialized = false;
		this->m_steam_initialized = false;
		this->m_module = NULL;
		this->fill_imgui_input_pointers();
		char sizebuf[256];
		sprintf(sizebuf, "width: %d, height: %d", width, height);
		this->debug_print("successfully created graphics context:\n	backend: " + graphics::get_backend_version() + "\n	" + sizebuf);
	}
	game::~game() {
		if (this->m_steam_initialized) this->shutdown_steam();
		if (this->m_debug_menus_initialized) debug::clean_up_imgui();
		delete this->m_sound_manager;
		delete this->m_menu_quad;
		delete this->m_menu_manager;
		delete this->m_scene;
		delete this->m_interpreter;
		delete this->m_model_registry;
		delete this->m_viewport;
		delete this->m_shader_registry;
		delete this->m_input_manager;
		delete this->m_module;
		delete this->m_script_registry;
	}
	void game::destroy() {
		DestroyWindow(this->m_window);
	}
	void game::update() {
		if (!this->m_show_cursor) SetCursor(NULL);
		this->m_frame++;
		this->m_menu_manager->update();
		this->m_input_manager->update_devices();
		this->m_scene->update();
	}
	void game::render() {
		this->m_menu_manager->draw();
		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		this->clear_screen();
		this->m_scene->render();
		if (this->m_menu_manager->menus_open()) {
			this->m_menu_quad->render();
		}
		if (this->m_debug_menus_initialized) debug::render_imgui(this);
	}
	unsigned int game::get_current_frame() {
		return this->m_frame;
	}
	input_manager* game::get_input_manager() {
		return this->m_input_manager;
	}
	model_registry* game::get_model_registry() {
		return this->m_model_registry;
	}
	ui::menu_manager* game::get_menu_manager() {
		return this->m_menu_manager;
	}
	long long __stdcall game::wndproc(HWND window, unsigned int msg, unsigned long long w_param, long long l_param) {
		if (ImGui_ImplWin32_WndProcHandler(window, msg, w_param, l_param))
			return true;
		switch (msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SIZE:
		{
			RECT r;
			GetWindowRect(window, &r);
			if (g_game) {
				long width = r.right - r.left;
				long height = r.bottom - r.top;
				g_game->m_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
				glViewport(0, 0, width, height);
			}
		}
			break;
		default:
			return DefWindowProc(window, msg, w_param, l_param);
			break;
		}
		return 0;
	}
	float game::get_aspect_ratio() {
		return this->m_aspect_ratio;
	}
	scene* game::get_scene() {
		return this->m_scene;
	}
	void game::add_model_desc(const model_registry::model_descriptor& desc) {
		this->m_descriptors.push_back(desc);
	}
	void game::load_models() {
		this->m_model_registry->load(this->m_descriptors);
	}
	HWND game::get_window() {
		return this->m_window;
	}
	sound::sound_manager* game::get_sound_manager() {
		return this->m_sound_manager;
	}
	lua_interpreter* game::get_lua_interpreter() {
		return this->m_interpreter;
	}
	bool& game::showing_cursor() {
		return this->m_show_cursor;
	}
	void game::show_cursor() {
		this->m_show_cursor = true;
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	void game::hide_cursor() {
		this->m_show_cursor = false;
		SetCursor(NULL);
	}
	void game::toggle_cursor() {
		if (this->m_show_cursor) this->hide_cursor();
		else this->show_cursor();
	}
	std::string game::get_steam_name() {
		if (this->m_steam_initialized) return std::string(SteamFriends()->GetPersonaName());
		else return "test player";
	}
	shader_registry* game::get_shader_registry() {
		return this->m_shader_registry;
	}
	script_registry* game::get_script_registry() {
		return this->m_script_registry;
	}
	void game::init_debug_menus() {
		debug::init_imgui(this->m_window);
		this->m_debug_menus_initialized = true;
	}
	void game::init_steam() {
		SteamAPI_Init();
		this->m_steam_initialized = true;
	}
	bool game::should_clip_cursor() {
		return this->m_clip_cursor;
	}
	void game::clip_cursor() {
		this->m_clip_cursor = true;
	}
	void game::unclip_cursor() {
		this->m_clip_cursor = false;
	}
	void game::toggle_cursor_clip() {
		this->m_clip_cursor = !this->m_clip_cursor;
	}
	void game::make_context_current() {
		opengl_viewport::use(this->m_viewport);
	}
	void game::swap_buffers() {
		this->m_viewport->swap_buffers();
	}
	void game::clear_screen() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	float game::get_timestep() {
		return ::get_current_time();
	}
	std::vector<std::string> game::get_command_line() {
		std::vector<std::string> arguments;
		std::string cmdline = GetCommandLineA();
		size_t offset = 0;
		do {
			size_t nextpos = cmdline.find("\" ", offset);
			if (nextpos == std::string::npos) nextpos = cmdline.find(' ', offset);
			else nextpos++;
			std::string arg;
			if (cmdline[offset] == '\"') {
				size_t start = offset + 1;
				size_t end;
				if (nextpos != std::string::npos) end = nextpos - 1;
				else end = cmdline.length() - 1;
				arg = cmdline.substr(start, end - start);
			} else arg = cmdline.substr(offset, nextpos - offset);
			offset = (nextpos == std::string::npos ? nextpos : nextpos + 1);
			arguments.push_back(arg);
		} while (offset != std::string::npos);
		return arguments;
	}
	void game::load_script_module(const std::string& dllpath) {
		delete this->m_module;
		this->m_module = new script_module(this->m_script_registry, dllpath);
	}
	void game::debug_print(const std::string& message) {
		std::string msg = message + "\n";
		this->debug_log << msg;
		OutputDebugStringA(msg.c_str());
	}
	std::list<rigidbody*>& game::get_rigidbody_list() {
		return this->m_rigidbodies;
	}
	std::string game::get_debug_log() {
		return this->debug_log.str();
	}
	void game::shutdown_steam() {
		SteamAPI_Shutdown();
	}
	static void int_input(const char* label, int* v) {
		ImGui::InputInt(label, v);
	}
	static void bool_input(const char* label, bool* v) {
		ImGui::Checkbox(label, v);
	}
	static void float_input(const char* label, float* v) {
		ImGui::DragFloat(label, v);
	}
	static void double_input(const char* label, double* v) {
		ImGui::InputDouble(label, v);
	}
	static void float2_input(const char* label, glm::vec2* v) {
		ImGui::DragFloat2(label, &v->x);
	}
	static void float3_input(const char* label, glm::vec3* v) {
		ImGui::DragFloat3(label, &v->x);
	}
	static void float4_input(const char* label, glm::vec4* v) {
		ImGui::DragFloat4(label, &v->x);
	}
	static void text_input(const char* label, std::string* v) {
		ImGui::InputText(label, v);
	}
	static void dropdown_input(const char* label, component::property_base::dropdown* value) {
		const std::vector<std::string>& std_items = value->get_items();
		std::vector<const char*> items;
		for (auto& str : std_items) {
			items.push_back(str.c_str());
		}
		ImGui::Combo(label, value->get_index_ptr(), items.data(), items.size());
	}
	static void readonly_input(const char* label, component::property_base::read_only_text* value) {
		ImGui::InputText(label, &value->get_text(), ImGuiInputTextFlags_ReadOnly);
	}
	void game::fill_imgui_input_pointers() {
		this->m_imgui_input_functions[typeid(int).hash_code()] = int_input;
		this->m_imgui_input_functions[typeid(bool).hash_code()] = bool_input;
		this->m_imgui_input_functions[typeid(float).hash_code()] = float_input;
		this->m_imgui_input_functions[typeid(double).hash_code()] = double_input;
		this->m_imgui_input_functions[typeid(std::string).hash_code()] = text_input;
		this->m_imgui_input_functions[typeid(glm::vec2).hash_code()] = float2_input;
		this->m_imgui_input_functions[typeid(glm::vec3).hash_code()] = float3_input;
		this->m_imgui_input_functions[typeid(glm::vec4).hash_code()] = float4_input;
		this->m_imgui_input_functions[typeid(component::property_base::dropdown).hash_code()] = dropdown_input;
		this->m_imgui_input_functions[typeid(component::property_base::read_only_text).hash_code()] = readonly_input;
	}
}