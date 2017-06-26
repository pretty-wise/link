<?php
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: google/protobuf/descriptor.proto

namespace Google\Protobuf\Internal;

use Google\Protobuf\Internal\GPBType;
use Google\Protobuf\Internal\GPBWire;
use Google\Protobuf\Internal\RepeatedField;
use Google\Protobuf\Internal\InputStream;

use Google\Protobuf\Internal\GPBUtil;

/**
 * <pre>
 * Describes a complete .proto file.
 * </pre>
 *
 * Protobuf type <code>google.protobuf.FileDescriptorProto</code>
 */
class FileDescriptorProto extends \Google\Protobuf\Internal\Message
{
    /**
     * <pre>
     * file name, relative to root of source tree
     * </pre>
     *
     * <code>optional string name = 1;</code>
     */
    private $name = '';
    private $has_name = false;
    /**
     * <pre>
     * e.g. "foo", "foo.bar", etc.
     * </pre>
     *
     * <code>optional string package = 2;</code>
     */
    private $package = '';
    private $has_package = false;
    /**
     * <pre>
     * Names of files imported by this file.
     * </pre>
     *
     * <code>repeated string dependency = 3;</code>
     */
    private $dependency;
    private $has_dependency = false;
    /**
     * <pre>
     * Indexes of the public imported files in the dependency list above.
     * </pre>
     *
     * <code>repeated int32 public_dependency = 10;</code>
     */
    private $public_dependency;
    private $has_public_dependency = false;
    /**
     * <pre>
     * Indexes of the weak imported files in the dependency list.
     * For Google-internal migration only. Do not use.
     * </pre>
     *
     * <code>repeated int32 weak_dependency = 11;</code>
     */
    private $weak_dependency;
    private $has_weak_dependency = false;
    /**
     * <pre>
     * All top-level definitions in this file.
     * </pre>
     *
     * <code>repeated .google.protobuf.DescriptorProto message_type = 4;</code>
     */
    private $message_type;
    private $has_message_type = false;
    /**
     * <code>repeated .google.protobuf.EnumDescriptorProto enum_type = 5;</code>
     */
    private $enum_type;
    private $has_enum_type = false;
    /**
     * <code>repeated .google.protobuf.ServiceDescriptorProto service = 6;</code>
     */
    private $service;
    private $has_service = false;
    /**
     * <code>repeated .google.protobuf.FieldDescriptorProto extension = 7;</code>
     */
    private $extension;
    private $has_extension = false;
    /**
     * <code>optional .google.protobuf.FileOptions options = 8;</code>
     */
    private $options = null;
    private $has_options = false;
    /**
     * <pre>
     * This field contains optional information about the original source code.
     * You may safely remove this entire field without harming runtime
     * functionality of the descriptors -- the information is needed only by
     * development tools.
     * </pre>
     *
     * <code>optional .google.protobuf.SourceCodeInfo source_code_info = 9;</code>
     */
    private $source_code_info = null;
    private $has_source_code_info = false;
    /**
     * <pre>
     * The syntax of the proto file.
     * The supported values are "proto2" and "proto3".
     * </pre>
     *
     * <code>optional string syntax = 12;</code>
     */
    private $syntax = '';
    private $has_syntax = false;

    public function __construct() {
        \GPBMetadata\Google\Protobuf\Internal\Descriptor::initOnce();
        parent::__construct();
    }

    /**
     * <pre>
     * file name, relative to root of source tree
     * </pre>
     *
     * <code>optional string name = 1;</code>
     */
    public function getName()
    {
        return $this->name;
    }

    /**
     * <pre>
     * file name, relative to root of source tree
     * </pre>
     *
     * <code>optional string name = 1;</code>
     */
    public function setName($var)
    {
        GPBUtil::checkString($var, True);
        $this->name = $var;
        $this->has_name = true;

        return $this;
    }

    public function hasName()
    {
        return $this->has_name;
    }

    /**
     * <pre>
     * e.g. "foo", "foo.bar", etc.
     * </pre>
     *
     * <code>optional string package = 2;</code>
     */
    public function getPackage()
    {
        return $this->package;
    }

    /**
     * <pre>
     * e.g. "foo", "foo.bar", etc.
     * </pre>
     *
     * <code>optional string package = 2;</code>
     */
    public function setPackage($var)
    {
        GPBUtil::checkString($var, True);
        $this->package = $var;
        $this->has_package = true;

        return $this;
    }

    public function hasPackage()
    {
        return $this->has_package;
    }

    /**
     * <pre>
     * Names of files imported by this file.
     * </pre>
     *
     * <code>repeated string dependency = 3;</code>
     */
    public function getDependency()
    {
        return $this->dependency;
    }

    /**
     * <pre>
     * Names of files imported by this file.
     * </pre>
     *
     * <code>repeated string dependency = 3;</code>
     */
    public function setDependency(&$var)
    {
        $arr = GPBUtil::checkRepeatedField($var, \Google\Protobuf\Internal\GPBType::STRING);
        $this->dependency = $arr;
        $this->has_dependency = true;

        return $this;
    }

    public function hasDependency()
    {
        return $this->has_dependency;
    }

    /**
     * <pre>
     * Indexes of the public imported files in the dependency list above.
     * </pre>
     *
     * <code>repeated int32 public_dependency = 10;</code>
     */
    public function getPublicDependency()
    {
        return $this->public_dependency;
    }

    /**
     * <pre>
     * Indexes of the public imported files in the dependency list above.
     * </pre>
     *
     * <code>repeated int32 public_dependency = 10;</code>
     */
    public function setPublicDependency(&$var)
    {
        $arr = GPBUtil::checkRepeatedField($var, \Google\Protobuf\Internal\GPBType::INT32);
        $this->public_dependency = $arr;
        $this->has_public_dependency = true;

        return $this;
    }

    public function hasPublicDependency()
    {
        return $this->has_public_dependency;
    }

    /**
     * <pre>
     * Indexes of the weak imported files in the dependency list.
     * For Google-internal migration only. Do not use.
     * </pre>
     *
     * <code>repeated int32 weak_dependency = 11;</code>
     */
    public function getWeakDependency()
    {
        return $this->weak_dependency;
    }

    /**
     * <pre>
     * Indexes of the weak imported files in the dependency list.
     * For Google-internal migration only. Do not use.
     * </pre>
     *
     * <code>repeated int32 weak_dependency = 11;</code>
     */
    public function setWeakDependency(&$var)
    {
        $arr = GPBUtil::checkRepeatedField($var, \Google\Protobuf\Internal\GPBType::INT32);
        $this->weak_dependency = $arr;
        $this->has_weak_dependency = true;

        return $this;
    }

    public function hasWeakDependency()
    {
        return $this->has_weak_dependency;
    }

    /**
     * <pre>
     * All top-level definitions in this file.
     * </pre>
     *
     * <code>repeated .google.protobuf.DescriptorProto message_type = 4;</code>
     */
    public function getMessageType()
    {
        return $this->message_type;
    }

    /**
     * <pre>
     * All top-level definitions in this file.
     * </pre>
     *
     * <code>repeated .google.protobuf.DescriptorProto message_type = 4;</code>
     */
    public function setMessageType(&$var)
    {
        $arr = GPBUtil::checkRepeatedField($var, \Google\Protobuf\Internal\GPBType::MESSAGE, \Google\Protobuf\Internal\DescriptorProto::class);
        $this->message_type = $arr;
        $this->has_message_type = true;

        return $this;
    }

    public function hasMessageType()
    {
        return $this->has_message_type;
    }

    /**
     * <code>repeated .google.protobuf.EnumDescriptorProto enum_type = 5;</code>
     */
    public function getEnumType()
    {
        return $this->enum_type;
    }

    /**
     * <code>repeated .google.protobuf.EnumDescriptorProto enum_type = 5;</code>
     */
    public function setEnumType(&$var)
    {
        $arr = GPBUtil::checkRepeatedField($var, \Google\Protobuf\Internal\GPBType::MESSAGE, \Google\Protobuf\Internal\EnumDescriptorProto::class);
        $this->enum_type = $arr;
        $this->has_enum_type = true;

        return $this;
    }

    public function hasEnumType()
    {
        return $this->has_enum_type;
    }

    /**
     * <code>repeated .google.protobuf.ServiceDescriptorProto service = 6;</code>
     */
    public function getService()
    {
        return $this->service;
    }

    /**
     * <code>repeated .google.protobuf.ServiceDescriptorProto service = 6;</code>
     */
    public function setService(&$var)
    {
        $arr = GPBUtil::checkRepeatedField($var, \Google\Protobuf\Internal\GPBType::MESSAGE, \Google\Protobuf\Internal\ServiceDescriptorProto::class);
        $this->service = $arr;
        $this->has_service = true;

        return $this;
    }

    public function hasService()
    {
        return $this->has_service;
    }

    /**
     * <code>repeated .google.protobuf.FieldDescriptorProto extension = 7;</code>
     */
    public function getExtension()
    {
        return $this->extension;
    }

    /**
     * <code>repeated .google.protobuf.FieldDescriptorProto extension = 7;</code>
     */
    public function setExtension(&$var)
    {
        $arr = GPBUtil::checkRepeatedField($var, \Google\Protobuf\Internal\GPBType::MESSAGE, \Google\Protobuf\Internal\FieldDescriptorProto::class);
        $this->extension = $arr;
        $this->has_extension = true;

        return $this;
    }

    public function hasExtension()
    {
        return $this->has_extension;
    }

    /**
     * <code>optional .google.protobuf.FileOptions options = 8;</code>
     */
    public function getOptions()
    {
        return $this->options;
    }

    /**
     * <code>optional .google.protobuf.FileOptions options = 8;</code>
     */
    public function setOptions(&$var)
    {
        GPBUtil::checkMessage($var, \Google\Protobuf\Internal\FileOptions::class);
        $this->options = $var;
        $this->has_options = true;

        return $this;
    }

    public function hasOptions()
    {
        return $this->has_options;
    }

    /**
     * <pre>
     * This field contains optional information about the original source code.
     * You may safely remove this entire field without harming runtime
     * functionality of the descriptors -- the information is needed only by
     * development tools.
     * </pre>
     *
     * <code>optional .google.protobuf.SourceCodeInfo source_code_info = 9;</code>
     */
    public function getSourceCodeInfo()
    {
        return $this->source_code_info;
    }

    /**
     * <pre>
     * This field contains optional information about the original source code.
     * You may safely remove this entire field without harming runtime
     * functionality of the descriptors -- the information is needed only by
     * development tools.
     * </pre>
     *
     * <code>optional .google.protobuf.SourceCodeInfo source_code_info = 9;</code>
     */
    public function setSourceCodeInfo(&$var)
    {
        GPBUtil::checkMessage($var, \Google\Protobuf\Internal\SourceCodeInfo::class);
        $this->source_code_info = $var;
        $this->has_source_code_info = true;

        return $this;
    }

    public function hasSourceCodeInfo()
    {
        return $this->has_source_code_info;
    }

    /**
     * <pre>
     * The syntax of the proto file.
     * The supported values are "proto2" and "proto3".
     * </pre>
     *
     * <code>optional string syntax = 12;</code>
     */
    public function getSyntax()
    {
        return $this->syntax;
    }

    /**
     * <pre>
     * The syntax of the proto file.
     * The supported values are "proto2" and "proto3".
     * </pre>
     *
     * <code>optional string syntax = 12;</code>
     */
    public function setSyntax($var)
    {
        GPBUtil::checkString($var, True);
        $this->syntax = $var;
        $this->has_syntax = true;

        return $this;
    }

    public function hasSyntax()
    {
        return $this->has_syntax;
    }

}

