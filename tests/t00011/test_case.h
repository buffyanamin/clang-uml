/**
 * tests/t00011/test_case.h
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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

TEST_CASE("t00011")
{
    using namespace clanguml::test;

    auto [config, db] = load_config("t00011");

    auto diagram = config.diagrams["t00011_class"];

    REQUIRE(diagram->name == "t00011_class");

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00011_class");

    CHECK_CLASS_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(IsClass(src, "A"));
        REQUIRE(IsClass(src, "B"));
        REQUIRE(!IsClass(src, "external::C"));
        REQUIRE(IsClassTemplate(src, "D<T>"));

        REQUIRE(IsAssociation<Public>(src, "B", "A"));
        REQUIRE(IsFriend<Public>(src, "A", "B"));
    });
}