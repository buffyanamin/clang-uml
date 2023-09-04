/**
 * @file src/common/generators/mermaid/generator.h
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "generator.h"

namespace clanguml::common::generators::mermaid {

std::string to_mermaid(relationship_t r, const std::string &style)
{
    switch (r) {
    case relationship_t::kOwnership:
    case relationship_t::kComposition:
        return style.empty() ? "*--" : fmt::format("*-[{}]-", style);
    case relationship_t::kAggregation:
        return style.empty() ? "o--" : fmt::format("o-[{}]-", style);
    case relationship_t::kContainment:
        return style.empty() ? "--" : fmt::format("-[{}]-", style);
    case relationship_t::kAssociation:
        return style.empty() ? "-->" : fmt::format("-[{}]->", style);
    case relationship_t::kInstantiation:
        return style.empty() ? "..|>" : fmt::format(".[{}].|>", style);
    case relationship_t::kFriendship:
        return style.empty() ? "<.." : fmt::format("<.[{}].", style);
    case relationship_t::kDependency:
        return style.empty() ? "..>" : fmt::format(".[{}].>", style);
    case relationship_t::kConstraint:
        return style.empty() ? "..>" : fmt::format(".[{}].>", style);
    case relationship_t::kAlias:
        return style.empty() ? ".." : fmt::format(".[{}].", style);
    default:
        return "";
    }
}

std::string to_mermaid(access_t scope)
{
    switch (scope) {
    case access_t::kPublic:
        return "+";
    case access_t::kProtected:
        return "#";
    case access_t::kPrivate:
        return "-";
    default:
        return "";
    }
}

std::string to_mermaid(message_t r)
{
    switch (r) {
    case message_t::kCall:
        return "->";
    case message_t::kReturn:
        return "-->";
    default:
        return "";
    }
}

} // namespace clanguml::common::generators::mermaid
