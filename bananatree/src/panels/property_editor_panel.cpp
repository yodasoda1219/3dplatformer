#include <thebanana.h>
#include "property_editor_panel.h"
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>
#include "scene_hierarchy_panel.h"
namespace bananatree {
	static void transform_menu(thebanana::gameobject* object, bool* open) {
		if (!object) return;
		ImGui::Begin("Transform menu", open);
		if (ImGui::CollapsingHeader("Move")) {
			static glm::vec3 m = glm::vec3(0.f);
			ImGui::InputFloat3("Motion values", &m.x);
			if (ImGui::Button("Apply")) {
				object->get_transform().move(m);
				m = glm::vec3(0.f);
			}
		}
		if (ImGui::CollapsingHeader("Translate")) {
			static glm::vec3 t = glm::vec3(0.f);
			ImGui::InputFloat3("Translation values", &t.x);
			if (ImGui::Button("Apply")) {
				object->get_transform().translate(t);
				t = glm::vec3(0.f);
			}
		}
		if (ImGui::CollapsingHeader("Rotate")) {
			static glm::vec3 r = glm::vec3(0.f);
			ImGui::InputFloat3("Rotation values", &r.x);
			if (ImGui::Button("Apply")) {
				if (fabs(r.x) > 0.001) object->get_transform().rotate(r.x, glm::vec3(1.f, 0.f, 0.f));
				if (fabs(r.y) > 0.001) object->get_transform().rotate(r.y, glm::vec3(0.f, 1.f, 0.f));
				if (fabs(r.z) > 0.001) object->get_transform().rotate(r.z, glm::vec3(0.f, 0.f, 1.f));
				r = glm::vec3(0.f);
			}
		}
		if (ImGui::CollapsingHeader("Scale")) {
			static glm::vec3 s = glm::vec3(1.f);
			ImGui::InputFloat3("Dilation values", &s.x);
			if (ImGui::Button("Apply")) {
				object->get_transform().scale(s);
				s = glm::vec3(1.f);
			}
		}
		ImGui::End();
	}
	property_editor_panel::property_editor_panel() {
		this->m_show_transform_menu = false;
		this->m_component_index = 0;
		this->m_hierarchy = NULL;
	}
	void property_editor_panel::render() {
		thebanana::gameobject* object = this->m_hierarchy->get_selected_object();
		if (this->m_show_transform_menu) transform_menu(object, &this->m_show_transform_menu);
		ImGui::Begin("Property Editor", &this->m_open);
		assert(this->m_hierarchy);
		if (object) {
			ImGui::InputText("Gameobject Nickname", &object->get_nickname());
			char buf[256];
			_ui64toa(object->get_uuid(), buf, 10);
			ImGui::Text("Gameobject UUID: %s", buf);
			if (ImGui::Button("Toggle transform menu")) this->m_show_transform_menu = !this->m_show_transform_menu;
			ImGui::Separator();
			ImGui::Text("Components:");
			for (size_t i = 0; i < object->get_number_components<thebanana::component>(); i++) {
				thebanana::component& c = object->get_component<thebanana::component>(i);
				const char* label = NULL;
				if (typeid(c).hash_code() == typeid(thebanana::tag_component).hash_code()) label = "Tag component";
				else if (typeid(c).hash_code() == typeid(thebanana::mesh_component).hash_code()) label = "Mesh component";
				else if (typeid(c).hash_code() == typeid(thebanana::animation_component).hash_code()) label = "Animation component";
				else if (typeid(c).hash_code() == typeid(thebanana::rigidbody).hash_code()) label = "Rigidbody";
				else if (typeid(c).hash_code() == typeid(thebanana::native_script_component).hash_code()) label = "Native script component";
				else if (typeid(c).hash_code() == typeid(thebanana::debug_component).hash_code()) label = "Debug component";
				assert(label);
				if (ImGui::CollapsingHeader(label)) {
					for (auto& p : c.get_properties()) {
						if (typeid(*p).hash_code() == typeid(thebanana::component::property<thebanana::gameobject*>).hash_code()) {
							// todo: wacky drap and drop stuff
						} else {
							p->draw();
						}
					}
				}
			}
			ImGui::Separator();
			static std::vector<const char*> combo_items = { "Tag component", "Mesh component", "Animation component", "Rigidbody", "Native script component" };
			ImGui::Combo("Type", &this->m_component_index, combo_items.data(), combo_items.size());
			ImGui::SameLine();
			if (ImGui::Button("Add component")) {
				switch (this->m_component_index) {
				case 0:
					object->add_component<thebanana::tag_component>();
					break;
				case 1:
					object->add_component<thebanana::mesh_component>();
					break;
				case 2:
					object->add_component<thebanana::animation_component>();
					break;
				case 3:
					object->add_component<thebanana::rigidbody>();
					break;
				case 4:
					object->add_component<thebanana::native_script_component>();
					break;
				}
			}
		} else {
			ImGui::Text("There is no Gameobject selected. You can select one from the scene's hierarchy panel.");
			if (this->m_show_transform_menu) this->m_show_transform_menu = false;
		}
		ImGui::End();
	}
	std::string property_editor_panel::get_menu_text() {
		return "Property Editor";
	}
	void property_editor_panel::set_hierarchy(scene_hierarchy_panel* hierarchy) {
		this->m_hierarchy = hierarchy;
	}
}