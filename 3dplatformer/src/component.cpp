#include "pch.h"
#include "component.h"
#include "gameobject.h"
component::component(gameobject* obj) : parent(obj) { }
void component::initialize() { }
void component::update() { }
void component::clean_up() { }
const component::properties_t& component::get_properties() const {
	return this->properties;
}
void component::add_property(property_base* p) {
	this->properties.push_back(std::unique_ptr<property_base>(p));
}
test_component::test_component(gameobject* obj) : component(obj) {
	this->add_property(new property<int>(42, "x"));
	this->add_property(new property<glm::vec3>(glm::vec3(1.f, -2.f, 3.4f), "y"));
	this->add_property(new property<std::string>("hello", "z"));
}
void test_component::print_properties() {
	property<int>* x = this->find_property<int>("x");
	if (x) {
		std::stringstream ss;
		ss << "x: " << *x->get_value() << "\n";
		std::string output = ss.str();
		OutputDebugStringA(output.c_str());
	}
	property<glm::vec3>* y = this->find_property<glm::vec3>("y");
	if (y) {
		std::stringstream ss;
		ss << "y: " << y->get_value()->x << ", " << y->get_value()->y << ", " << y->get_value()->z << "\n";
		std::string output = ss.str();
		OutputDebugStringA(output.c_str());
	}
	property<std::string>* z = this->find_property<std::string>("z");
	if (z) {
		std::stringstream ss;
		ss << "z: " << *z->get_value() << "\n";
		std::string output = ss.str();
		OutputDebugStringA(output.c_str());
	}
}
component::property_base::property_base(const std::string& name, size_t size) {
	this->ptr = malloc(size);
	this->name = name;
}
const std::string& component::property_base::get_name() {
	return this->name;
}
component::property_base::~property_base() { }
void* component::property_base::get_ptr() {
	return this->ptr;
}
void component::property<int>::draw() const {
	ImGui::InputInt(this->name.c_str(), this->value);
}
void component::property<float>::draw() const {
	ImGui::InputFloat(this->name.c_str(), this->value);
}
void component::property<double>::draw() const {
	ImGui::InputDouble(this->name.c_str(), this->value);
}
void component::property<std::string>::draw() const {
	ImGui::InputText(this->name.c_str(), this->value);
}
void component::property<glm::vec2>::draw() const {
	ImGui::InputFloat2(this->name.c_str(), &this->value->x);
}
void component::property<glm::vec3>::draw() const {
	ImGui::InputFloat3(this->name.c_str(), &this->value->x);
}
void component::property<glm::vec4>::draw() const {
	ImGui::InputFloat4(this->name.c_str(), &this->value->x);
}