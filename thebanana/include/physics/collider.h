#pragma once
namespace thebanana {
	class rigidbody;
	class gameobject;
	class collider {
	public:
		collider(rigidbody* rb);
		virtual ~collider();
		virtual bool detect_collision(rigidbody* other) = 0;
		virtual void on_collision(gameobject* other) = 0;
	protected:
		rigidbody* parent;
	};
	//https://gist.github.com/mlfarrell/5b1d77326fb6f95c4fa7d9cf8622e992
	class mlfarrel_model : public collider {
	public:
		mlfarrel_model(rigidbody* rb);
		virtual bool detect_collision(rigidbody* other) override;
		virtual void on_collision(gameobject* other) override;
		mlfarrel_model& set_radius(float radius);
		mlfarrel_model& set_origin_offset(glm::vec3 offset);
		float get_radius() const;
		glm::vec3 get_origin_offset() const;
	private:
		glm::vec3 origin_offset;
		glm::vec3 last_frame_position;
		float radius;
	};
}