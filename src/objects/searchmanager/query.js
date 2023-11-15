/**
 * @fileOverview Query class
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
let queryIdCounter = 0;
hbbtv.objects.Query = (function() {
    const prototype = {};
    const privates = new WeakMap();

    prototype.and = function(query) {
        return createAndQuery(this, query);
    };

    prototype.or = function(query) {
        return createOrQuery(this, query);
    };

    prototype.not = function() {
        return createNotQuery(this);
    };

    prototype.toJSON = function(recursive) {
        const p = privates.get(this);
        let result;
        if (p.queryData.operation === 'IDENTITY') {
            result = {
                comparison: p.queryData.op1.comparison,
                field: p.queryData.op1.field,
                value: p.queryData.op1.value,
            };
        } else if (p.queryData.operation === 'NOT') {
            result = {
                arguments: [p.queryData.op1.toJSON(true)],
                operation: p.queryData.operation,
            };
        } else {
            result = {
                arguments: [p.queryData.op1.toJSON(true), p.queryData.op2.toJSON(true)],
                operation: p.queryData.operation,
            };
        }

        if (recursive !== true) {
            result.queryId = p.queryId;
        }
        return result;
    };

    prototype.queryId = function() {
        return privates.get(this).queryId;
    };

    // Private method to get a copy of the query data
    function cloneQueryData() {
        return Object.assign({}, privates.get(this).queryData);
    }

    function initialise(queryData) {
        privates.set(this, {});
        const p = privates.get(this);
        p.queryId = queryIdCounter++;
        p.queryData = queryData;
    }

    function createAndQuery(op1, op2) {
        const query = Object.create(hbbtv.objects.Query.prototype);
        hbbtv.objects.Query.initialise.call(query, {
            operation: 'AND',
            op1: hbbtv.objects.createQuery(op1),
            op2: hbbtv.objects.createQuery(op2),
        });
        return query;
    }

    function createOrQuery(op1, op2) {
        const query = Object.create(hbbtv.objects.Query.prototype);
        hbbtv.objects.Query.initialise.call(query, {
            operation: 'OR',
            op1: hbbtv.objects.createQuery(op1),
            op2: hbbtv.objects.createQuery(op2),
        });
        return query;
    }

    function createNotQuery(op1) {
        const query = Object.create(hbbtv.objects.Query.prototype);
        hbbtv.objects.Query.initialise.call(query, {
            operation: 'NOT',
            op1: hbbtv.objects.createQuery(op1),
        });
        return query;
    }

    return {
        prototype: prototype,
        initialise: initialise,
        cloneQueryData: cloneQueryData,
    };
})();

hbbtv.objects.createQuery = function(field, comparison, value) {
    const query = Object.create(hbbtv.objects.Query.prototype);
    if (typeof field === 'string') {
        hbbtv.objects.Query.initialise.call(query, {
            operation: 'IDENTITY',
            op1: {
                field: field,
                comparison: comparison,
                value: value,
            },
        });
    } else {
        hbbtv.objects.Query.initialise.call(query, hbbtv.objects.Query.cloneQueryData.call(field));
    }
    return query;
};