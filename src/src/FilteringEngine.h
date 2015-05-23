//
// Created by Adam Kosiorek on 22.05.15.
//

#ifndef OPTICAL_FLOW_FILTERINGENGINE_H
#define OPTICAL_FLOW_FILTERINGENGINE_H

#include <memory>

#include "EventSlice.h"
#include "FlowSlice.h"
#include "IFilterFactory.h"
#include "FourierPadder.h"
#include "IFourierTransformer.h"


template<template <class> class InputBufferT, template <class> class OutputBufferT = InputBufferT>
class FilteringEngine {
public:
    typedef FourierPadder<128, 21> PadderT;

    typedef InputBufferT<std::shared_ptr<EventSlice>> EventQueueT;
    typedef OutputBufferT<std::shared_ptr<FlowSlice>> FlowQueueT;

    FilteringEngine(std::unique_ptr<IFilterFactory> factory,
                    std::unique_ptr<PadderT> padder,
                    std::unique_ptr<IFourierTransformer> transformer)

            : factory_(std::move(factory)),
              padder_(std::move(padder)),
              transformer_(std::move(transformer)),
              timeSteps_(0) {

        //TODO wait for dynamic reference-argument-accepting FourierPadder
//        factory-setFilterTransformer(
//                [this](const Eigen::MatrixXf& filter) {
//
//                    auto nullDeleter = [](const Eigen::MatrixXf*) {};
//
//                    auto ptr = std::shared_ptr<Eigen::MatrixXf>(&filter, nullDeleter);
//                    this->padder_->padData(ptr);
//                });
    }

    void setInputBuffer(std::shared_ptr<EventQueueT> buffer) {
        this->inputBuffer_ = buffer;
    }
    void setOutputBuffer(std::shared_ptr<FlowQueueT> buffer) {
        this->outputBuffer_ = buffer;
    }

    int timeSteps() {
        return timeSteps_;
    }

    bool isInitialized() {
        LOG(ERROR) << " size = " << eventStream_.size();
        return timeSteps_ != 0 && eventStream_.size() >= timeSteps_;
    }

    bool hasInput() {
        return inputBuffer_ && !inputBuffer_->empty();
    }

    bool hasOutput() {
        return outputBuffer_ && !outputBuffer_->empty();
    }

    void addFilter(int angle) {
        auto it = std::find_if(std::begin(filters_), std::end(filters_),
        [angle](std::shared_ptr<Filter> filter) {
            return filter->angle() == angle;
        });

        if(it == filters_.end()) {
            auto filter = factory_->createFilter(angle);
            filters_.push_back(filter);
            timeSteps_ = filter->numSlices();
            responseTemplates_.emplace_back(filter->xSize(), filter->ySize());
            responseTemplates_[responseTemplates_.size() - 1].setZero();
        }
    };

    void filter(std::shared_ptr<EventSlice> slice) {

        bool wasInitialized = isInitialized();
        transformAndEnqueue(slice);

        // may the magic happen
        if(wasInitialized) {

            eventStream_.pop_front();

            int sliceIndex = 0;
            auto eventIt = eventStream_.begin();
            for(; eventIt != eventStream_.end(); ++sliceIndex, ++eventIt) {
                const auto& eventSlice = **eventIt;
                for(int filterIndex = 0; filterIndex < filters_.size(); ++filterIndex) {

                    // iterate over event slices and filters/filter slices transforming,
                    // multiplying, summing
                }
            }
            // reverse transform
            // weight by filter angles
            // sum up
            // put the result to the output queue
        }
    }

private:
    void transformAndEnqueue(std::shared_ptr<EventSlice>& eventSlice) {
        eventStream_.emplace_back();

        auto padded = padder_->padData(eventSlice);
    }

private:
    std::shared_ptr<EventQueueT> inputBuffer_;
    std::shared_ptr<FlowQueueT> outputBuffer_;

    std::unique_ptr<IFilterFactory> factory_;
    std::unique_ptr<PadderT> padder_;
    std::unique_ptr<IFourierTransformer> transformer_;
    int timeSteps_;

    std::vector<std::shared_ptr<Filter>> filters_;
    std::vector<Eigen::MatrixXf> responseTemplates_;

    // use deque instead of a queue for it's iterator
    std::deque<std::shared_ptr<PadderT::FourierMatrix>> eventStream_;
};


#endif //OPTICAL_FLOW_FILTERINGENGINE_H
