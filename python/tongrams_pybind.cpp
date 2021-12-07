#include <pybind11/pybind11.h>

#include "lm_types.hpp"
#include "score.hpp"
#include "utils/util.hpp"

#include <iostream>

namespace py = pybind11;
using namespace tongrams;

template <typename Model>
static int load_from_binary(Model* model, const char* binary_filename) {
    std::string model_string_type = util::get_model_type(binary_filename);
    std::cout << "Loading data structure type: " << model_string_type << "\n";
    size_t file_size = util::load(*(model), binary_filename);
    std::cout << "\tTotal bytes: " << file_size << "\n";
    std::cout << "\tTotal ngrams: " << model->size() << "\n";
    std::cout << "\tBytes per gram: " << double(file_size) / model->size()
              << std::endl;
    return 0;
}

typedef ef_trie_IC_ranks_count_lm count_lm_type;
typedef ef_trie_prob_lm prob_lm_type;

struct CountModel {
    std::unique_ptr<count_lm_type> model;
    CountModel(const char* binary_filename) {
        model = std::make_unique<count_lm_type>();

        std::cout << load_from_binary<count_lm_type>(this->model.get(),
                                                     binary_filename)
                  << std::endl;
    }
    uint64_t lookup(const char* ngram) {
        stl_string_adaptor adaptor;
        uint64_t count = this->model->lookup(ngram, adaptor);
        if (count == global::not_found) {
            return 0;
        } else {
            return count;
        }
    }
};

struct ProbModel {
    std::unique_ptr<prob_lm_type> model;
    ProbModel(const char* binary_filename) {
        model = std::make_unique<prob_lm_type>();
        std::cout << load_from_binary<prob_lm_type>(this->model.get(),
                                                    binary_filename)
                  << std::endl;
    }
    std::pair<float, uint64_t> score_corpus(const char* corpus_filename) {
        text_lines corpus(corpus_filename);
        auto state = this->model->state();
        float tot_log10_prob = 0.0;
        while (!corpus.end_of_file()) {
            state.init();
            float sentence_log10_prob = 0.0;
            corpus.begin_line();
            while (!corpus.end_of_line()) {
                auto word = corpus.next_word();
                bool is_OOV = false;
                float log10_prob = this->model->score(state, word, is_OOV);
                sentence_log10_prob += log10_prob;
            }
            tot_log10_prob += sentence_log10_prob;
        }
        return {tot_log10_prob, corpus.num_words()};
    }

    std::pair<float, uint64_t> score_sentence(const char* sentence,
                                              uint64_t size) {
        uint8_t const* ptr = reinterpret_cast<uint8_t const*>(sentence);
        forward_byte_range_iterator it;
        it.init({ptr, ptr + size});
        auto state = this->model->state();
        state.init();
        float sentence_log10_prob = 0.0;
        uint64_t words = 0;
        while (it.has_next()) {
            auto word = it.next();
            bool is_OOV = false;
            float log10_prob = this->model->score(state, word, is_OOV);
            sentence_log10_prob += log10_prob;
            ++words;
        }
        return {sentence_log10_prob, words};
    }
};
PYBIND11_MODULE(tongrams, m) {
    py::class_<CountModel>(m, "CountModel")
        .def(py::init<const char*>())
        .def("lookup", &CountModel::lookup);

    py::class_<ProbModel>(m, "ProbModel")
        .def(py::init<const char*>())
        .def("score_corpus", &ProbModel::score_corpus)
        .def("score_sentence", &ProbModel::score_sentence);
}