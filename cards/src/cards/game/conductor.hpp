#pragma once
#include "cards/game/board.hpp"
#include "cards/game/evaluator.hpp"
#include "cards/services.hpp"
#include <variant>

namespace cards::game {
struct GameInfo {
	std::span<Denomination const> custom_deck{};
	klib::Ptr<Evaluator::IRanker const> custom_ranker{};
	klib::Ptr<Tally::IPointer const> custom_pointer{};
};

class Conductor : public le::IDrawable {
  public:
	enum class State : std::int8_t { Idle, Dealing, WaitingForSubmit, Submitting, Discarding };
	inline static auto const state_name_map = klib::EnumNameMap<State>{
		{State::Idle, "Idle"},
		{State::Dealing, "Dealing"},
		{State::WaitingForSubmit, "WaitingForSubmit"},
		{State::Submitting, "Submitting"},
		{State::Discarding, "Discarding"},
	};

	explicit Conductor(gsl::not_null<IServices const*> services, gsl::not_null<Board*> board);

	[[nodiscard]] auto get_state() const -> State { return m_state; }

	void draw(le::IRenderer& renderer) const final;

	void start_game(GameInfo const& game_info = {});

	void tick(kvf::Seconds dt);

  private:
	struct Deal {
		std::vector<Denomination> denominations{};

		Seat next_seat{};
		kvf::Seconds remain{};
		bool sfx_played{};
	};

	struct Submit {
		void draw(le::IRenderer& renderer) const;

		Seat seat;
		Card card;

		glm::vec2 src{};
		glm::vec2 dst{};
		kvf::Seconds elapsed{};
	};

	struct Discard {
		void draw(le::IRenderer& renderer) const;

		Round round;
		PerSeat<glm::vec2> offsets{};
		glm::vec2 position{};

		glm::vec2 dst{};
		kvf::Seconds start_remain{};
		kvf::Seconds elapsed{};
		bool sfx_played{};
	};

	using Anim = std::variant<std::monostate, Deal, Submit, Discard>;

	void tick_deal(kvf::Seconds dt);
	void tick_wait_submit(kvf::Seconds dt);
	void tick_submit(kvf::Seconds dt);
	void tick_discard(kvf::Seconds dt);
	void shared_tick(kvf::Seconds dt);

	void start_deal();
	void finish_deal();
	void start_submit(Card card);
	void finish_submit();
	void start_discard();
	void finish_discard();

	void play_sfx_if(klib::Ptr<le::IAudioBuffer const> buffer) const;

	gsl::not_null<IServices const*> m_services;
	gsl::not_null<Timings const*> m_timings;

	gsl::not_null<Board*> m_board;

	GameInfo m_game_info{};

	State m_state{};
	Anim m_anim{};
};
} // namespace cards::game
