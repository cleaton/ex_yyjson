defmodule ExYyjsonTest do
  use ExUnit.Case
  doctest ExYyjson

  @json """
  {
    "meta": {
     "limit": 1000,
     "offset": 0,
     "total_count": 12063
    },
    "object": {
      "bill_resolution_type": "bill",
      "bill_type": "house_bill",
      "bill_type_label": "H.R.",
      "committee_reports": []
    }
  }
  """

  test "decode" do
    assert ExYyjson.decode(@json) == :world
  end

  test "encode" do
    #term = %{
    #  "meta": %{
    #    "limit": 1000,
    #    "offset": 0,
    #    "total_count": 12063
    #  },
    #  "object": %{
    #    "bill_resolution_type": "bill",
    #    "bill_type": "house_bill",
    #    "bill_type_label": "H.R.",
    #    "committee_reports": []
    #  }
    #}
    term = ["hi", "there", "world"]
    assert ExYyjson.encode(term) == @json
  end

  test "encode & decode bench files" do
    inputs = [
      "Benchee",
      "Blockchain",
      "GeoJSON",
      "Giphy",
      "GitHub",
      "GovTrack",
      "Issue 90",
      "JSON Generator (Pretty)",
      "JSON Generator",
      "Pokedex",
      "Reddit",
      "UTF-8 escaped",
      "UTF-8 unescaped"
    ]

    path = "#{__DIR__}/../bench/data"
    jsons = for name <- inputs, into: %{} do
      name
      |> String.downcase()
      |> String.replace(~r/([^\w]|-|_)+/, "-")
      |> String.trim("-")
      |> (&"#{path}/#{&1}.json").()
      |> Path.expand(__DIR__)
      |> File.read!()
      |> (&{name, &1}).()
    end

    for {name, json} <- jsons do
      term = ExYyjson.decode!(json)
      IO.inspect(term)
      json_again = ExYyjson.encode(term)
      IO.inspect(json_again)
    end
  end
end
